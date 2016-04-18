/**
* @file     demo-sts-api.c
* @author   Mark Zieg <mark.zieg@oceanoptics.com>
* @date     2014-07-22
*
* Demonstrate how to communicate via the STS / Ocean Binary Protocol (OBP)
* API, using either USB (via SeaBreeze) or raw serial comms.
*
* Serial functions borrowed from:
* http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
*
* Please see STS and Spark Data Sheets for Ocean Binary Protocol API:
* http://oceanoptics.com//wp-content/uploads/STS-Data-Sheet.pdf
*
* LICENSE:
*
* SeaBreeze Copyright (C) 2014, Ocean Optics Inc
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject
* to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// uncomment this line to use RS232 rather than SeaBreeze
//#define RS232

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#ifdef RS232
#include <termios.h>
#include <fcntl.h>
#else
#include "api/SeaBreezeWrapper.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Preprocessor Macros
////////////////////////////////////////////////////////////////////////////////

// just a few of the many commands available within OBP
#define OBP_MESSAGE_RESET_DEFAULTS          0x00000001L
#define OBP_MESSAGE_GET_FLOW_CONTROL        0x00000804L
#define OBP_MESSAGE_GET_HARDWARE_VERSION    0x00000080L
#define OBP_MESSAGE_GET_FIRMWARE_VERSION    0x00000090L
#define OBP_MESSAGE_GET_SERIAL_NUMBER       0x00000100L
#define OBP_MESSAGE_GET_SERIAL_NUMBER_LEN   0x00000101L
#define OBP_MESSAGE_GET_USER_STRING_COUNT   0x00000300L
#define OBP_MESSAGE_GET_USER_STRING_LEN     0x00000301L
#define OBP_MESSAGE_GET_USER_STRING         0x00000302L
#define OBP_MESSAGE_SET_INTEGRATION_TIME    0x00110010L
#define OBP_MESSAGE_GET_CORRECTED_SPECTRUM  0x00101000L
#define OBP_MESSAGE_GET_RAW_SPECTRUM        0x00101100L
#define OBP_MESSAGE_SET_SUBSPECTRUM_SPEC    0x00102010L
#define OBP_MESSAGE_GET_SUBSPECTRUM_SPEC    0x00102000L
#define OBP_MESSAGE_GET_SUBSPECTRUM         0x00102080L
#define OBP_MESSAGE_GET_BOXCAR_WIDTH        0x00121000L
#define OBP_MESSAGE_SET_BOXCAR_WIDTH        0x00121010L
#define OBP_MESSAGE_GET_BINNING_FACTOR      0x00110280L
#define OBP_MESSAGE_SET_BINNING_FACTOR      0x00110290L
#define OBP_MESSAGE_GET_WL_COEFF_COUNT      0x00180100L
#define OBP_MESSAGE_GET_WL_COEFF            0x00180101L
#define OBP_MESSAGE_SET_WL_COEFF            0x00180111L
#define OBP_MESSAGE_GET_NLC_COEFF_COUNT     0x00181100L
#define OBP_MESSAGE_GET_NLC_COEFF           0x00181101L
#define OBP_MESSAGE_SET_NLC_COEFF           0x00181111L

// convert 2-byte LSB-MSB to native
#define LITTLE_ENDIAN_SHORT(base) (((base)[1] << 8) | (base)[0])

// convert 4-byte LSB-MSB to native
#define LITTLE_ENDIAN_WORD(base) (((base)[3] << 24) | ((base)[2] << 16) | ((base)[1] << 8) | (base)[0])

////////////////////////////////////////////////////////////////////////////////
// Data Types
////////////////////////////////////////////////////////////////////////////////

// This is an internal data type, used only within this sample code, to represent
// an atomic "request-response" exchange, as typically occurs with OBP transactions.
typedef struct OBPExchange_s
{
    unsigned int   message_type;        // (input)  OBP message code
    unsigned int   request_len;         // (input)  how much request parameter data to send as "OBP immediate"
    unsigned char *request;             // (input)  pointer to request parameter data
    unsigned int   response_len;        // (input)  max size of response data (output_data allocated size)
    unsigned char *response;            // (output) pointer to where response data should be written
    unsigned int   extra_response_len;  // (input)  how many bytes of non-immediate response expected (beyond 16-byte immediate)
    unsigned int   actual_response_len; // (output) how much data actually came back in response
} OBPExchange;

// Taken directly from page 16 of the STS Data Sheet
typedef struct OBPHeader_s
{                                       // byte offset
    unsigned char  start_bytes[2];      // 0-1: 0xC1, 0xC0
    unsigned short protocol_version;    // 2-3: 0x1000
    unsigned short flags;               // 4-5
    unsigned short err_no;              // 6-7
    unsigned int   message_type;        // 8-11
    unsigned int   regarding;           // 12-15
    unsigned char  reserved[6];         // 16-21
    unsigned char  checksum_type;       // 22
    unsigned char  immediate_data_len;  // 23
    unsigned char  immediate_data[16];  // 24-39
    unsigned int   bytes_remaining;     // 40-43
} OBPHeader;                            // 44 bytes total

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

int enable_verbose_com = 0;             // debug serial comms to stdout
int enable_verbose_obp = 0;             // debug protocol internals to stdout
unsigned int opt_integration_time = 0;  // integration time to set
int opt_boxcar = 0;                     // boxcar width size
int opt_binning = 0;                    // pixel binning factor
int opt_info = 0;                       // read-only info (don't set integration time)
int opt_partial = 0;                    // get partial corrected spectra
int opt_raw = 0;                        // get raw spectrum immediate
int opt_corrected = 0;                  // get corrected spectrum immediate


#ifdef RS232
const int BAUD_RATE = 9600;
char *serial_path = NULL;               // "/dev/ttyS0" or similar
int serial_fd = -1;                     // result of open()
#else
const unsigned char REQUEST_ENDPOINT  = 0x01;
const unsigned char RESPONSE_ENDPOINT = 0x81;
const int spec_index = 0;
int error = 0;
#endif

////////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////////

void dumpOBP(const char *label, unsigned char *msg, int len);

////////////////////////////////////////////////////////////////////////////////
// RS232 implementation
////////////////////////////////////////////////////////////////////////////////

#ifdef RS232
int set_interface_attribs(int fd, int speed, int parity)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty))
    {
        printf("%s: error %d from tcgetattr\n", __FUNCTION__, errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars

    // disable IGNBRK for mismatched speed tests;
    // otherwise receive break as \000 chars
    tty.c_iflag    &= ~IGNBRK;  // disable break processing
    tty.c_lflag     = 0;        // no signaling, echo, or processing
    tty.c_oflag     = 0;        // no remapping or delays
    tty.c_cc[VMIN]  = 0;        // read doesn't block
    tty.c_cc[VTIME] = 5;        // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    tty.c_cflag |=  (CLOCAL | CREAD);       // ignore modem ctrl, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |=  parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        printf("%s: error %d from tcsetattr", __FUNCTION__, errno);
        return -1;
    }
    return 0;
}

void set_blocking(int fd, int should_block)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf("%s: error %d from tggetattr\n", __FUNCTION__, errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf("%s: error %d setting term attributes\n", __FUNCTION__, errno);
    }
}

// computers run faster than 9600 baud, so wait for comms or FIFOs may overflow
void waitForTransmission(int bytesToTransfer)
{
    const double MICROSEC_PER_BIT = 1000000L / BAUD_RATE;

    int bitsToTransfer = 10 * bytesToTransfer; // 8-N-1 = startBit + octet + stopBit
    unsigned long microsecToSleep = (unsigned long) (bitsToTransfer * MICROSEC_PER_BIT + 0.5);

    usleep(microsecToSleep);
}
#endif

// dump a block of bytes in hex and ASCII for debugging
void dumpHex(const char *label, const char *prefix, unsigned char *msg, size_t len)
{
    printf("%s %d bytes:\n", label, (int) len);
    size_t offset = 0;
    do
    {
        char txt[32];
        memset(txt, 32, 32); // initialize 32-byte buffer with spaces
        txt[31] = 0;

        // print block of 16 bytes as hex and ASCII
        printf("%s %04d: ", prefix, (int) offset);
        for (int i = 0; i < 16; i++)
        {
            int pos = i + offset;
            if (pos < len)
            {
                unsigned char c = 0;
                if (pos < len)
                    c = msg[pos];
                printf("0x%02x ", c);

                if (32 <= c && c < 127)
                    txt[i * 2] = c;
                else
                    txt[i * 2] = '.';
            }
            else
            {
                printf("     ");
            }
        }
        printf(" %s\n", txt);

        // advance to next block
        offset += 16;
    } while (offset < len);
}

// send a buffer to the spectrometer
void write_buffer(unsigned char *request, size_t len)
{
    if (enable_verbose_obp)
        dumpOBP("writing", request, len);

    if (enable_verbose_com)
        dumpHex("writing", "  [COM] >>", request, len);

#ifdef RS232
    write(serial_fd, request, len);
    waitForTransmission(len);
#else
    seabreeze_write_usb(spec_index, &error, REQUEST_ENDPOINT, request, len);
#endif
}

// read a buffer from the spectrometer
int read_buffer(unsigned char *response, size_t len)
{
    int total_bytes_read = 0;

#ifdef RS232
    waitForTransmission(len);
    int bytes_left_to_read = len;
    do
    {
        int bytes_read = read(serial_fd, response + total_bytes_read, bytes_left_to_read);
        if (enable_verbose_com)
            dumpHex("reading", "  [COM] <<", response + total_bytes_read, bytes_read);

        total_bytes_read += bytes_read;
        bytes_left_to_read -= bytes_read;
    } while (total_bytes_read < len);

    if (bytes_left_to_read != 0)
        printf("%s: bytes_left_to_read = %d\n", __FUNCTION__, bytes_left_to_read);
#else
    total_bytes_read = seabreeze_read_usb(spec_index, &error, RESPONSE_ENDPOINT, response, len);
    if (enable_verbose_com)
        dumpHex("reading", "  [COM] <<", response, total_bytes_read);
#endif

    if (enable_verbose_obp)
        dumpOBP("read", response, total_bytes_read);

    return total_bytes_read;
}

////////////////////////////////////////////////////////////////////////////////
// Ocean Binary Protocol implementation
////////////////////////////////////////////////////////////////////////////////

// given a serialized 44-byte OBP header, populate the struct for easy referencing
void deserializeHeader(OBPHeader *header, unsigned char *msg)
{
    if (!header || !msg)
    {
        printf("ERROR: can't deserialize to/from NULL\n");
        return;
    }

    for (int i = 0; i < 2; i++)
        header->start_bytes[i]  = msg[0+i];

    header->protocol_version    = LITTLE_ENDIAN_SHORT(msg +  2);
    header->flags               = LITTLE_ENDIAN_SHORT(msg +  4);
    header->err_no              = LITTLE_ENDIAN_SHORT(msg +  6);
    header->message_type        = LITTLE_ENDIAN_WORD (msg +  8);
    header->regarding           = LITTLE_ENDIAN_WORD (msg + 12);

    for (int i = 0; i < 6; i++)
        header->reserved[i]     = msg[16+i];

    header->checksum_type       = msg[22];
    header->immediate_data_len  = msg[23];

    for (int i = 0; i < 16; i++)
        header->immediate_data[i] = msg[24+i];

    header->bytes_remaining     = LITTLE_ENDIAN_WORD (msg + 40);
}

// convert an OBP header.err_no to human-readable string
const char* getOBPError(unsigned err_no)
{
    switch (err_no)
    {
        case   0: return "Success";
        case   1: return "Invalid/unsupported protocol";
        case   2: return "Unknown message type";
        case   3: return "Bad checksum";
        case   4: return "Message too large";
        case   5: return "Payload length does not match message type";
        case   6: return "Payload data invalid";
        case   7: return "Device not ready for given message";
        case   8: return "Unknown checksum type";
        case   9: return "Device reset unexpectedly";
        case  10: return "Too many buses";
        case  11: return "Out of memory";
        case  12: return "Desired information does not exist";
        case  13: return "Internal device error";
        case 100: return "Could not decrypt properly";
        case 101: return "Firmware layout invalid";
        case 102: return "Data packet wrong size";
        case 103: return "Hardware revision not compatible with firmware";
        case 104: return "Existing flash map not compatible with firmware";
        case 255: return "Operation/Response deferred.  Operation will take some time to complete.";
    }

    printf("%s: unknown OPB error code (%d)\n", __FUNCTION__, err_no);
    return "UNKNOWN OBP ERROR CODE";
}

// convert header.flags to human-readable string
const char *getOBPFlags(unsigned int flags)
{
    static char s[256];

    memset(s, 0, sizeof(s));

    if (flags & 0x0001) strncat(s, " | DEVICE:RESPONSE_TO_REQUEST" , (int) sizeof(s) - 1);
    if (flags & 0x0002) strncat(s, " | DEVICE:ACK"                 , (int) sizeof(s) - 1);
    if (flags & 0x0004) strncat(s, " | HOST:ACK_REQUESTED"         , (int) sizeof(s) - 1);
    if (flags & 0x0008) strncat(s, " | DEVICE:NACK"                , (int) sizeof(s) - 1);
    if (flags & 0x0010) strncat(s, " | DEVICE:ERROR"               , (int) sizeof(s) - 1);
    if (flags & 0x0020) strncat(s, " | DEVICE:DEPRECATED_PROTOCOL" , (int) sizeof(s) - 1);
    if (flags > 0x003f) strncat(s, " | UNKNOWN"                    , (int) sizeof(s) - 1);

    // truncate leading delimiter
    return s + 3;
}

// given a serialized OBP message (header, payload, checksum and footer),
// render to console for debugging
void dumpOBP(const char *label, unsigned char *msg, int len)
{
    printf("%s the following OBP message:\n", label);

    if (len < 44)
    {
        printf("  [OBP] message doesn't have room for header!\n");
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    // header
    ////////////////////////////////////////////////////////////////////////////

    OBPHeader header;
    deserializeHeader(&header, msg);
    printf("  [OBP] Header: (%zu bytes)\n", sizeof(header));
    printf("  [OBP]   start_bytes        = 0x%02x 0x%02x\n", header.start_bytes[0], header.start_bytes[1]);
    printf("  [OBP]   protocol_version   = 0x%04x\n", header.protocol_version);
    printf("  [OBP]   flags              = 0x%04x (%s)\n", header.flags, getOBPFlags(header.flags));
    printf("  [OBP]   err_no             = 0x%04x (%s)\n", header.err_no, getOBPError(header.err_no));
    printf("  [OBP]   message_type       = 0x%08x\n", header.message_type);
    printf("  [OBP]   regarding          = 0x%08x\n", header.regarding);

    printf("  [OBP]   reserved           =");
    for (int i = 0; i < 6; i++)
        printf(" 0x%02x", header.reserved[i]);
    printf("\n");

    printf("  [OBP]   checksum_type      = 0x%02x\n", header.checksum_type);
    printf("  [OBP]   immediate_data_len = 0x%02x (%d bytes)\n", header.immediate_data_len, header.immediate_data_len);

    printf("  [OBP]   immediate_data     =");
    for (int i = 0; i < 16; i++)
        printf(" 0x%02x", header.immediate_data[i]);
    printf("\n");

    printf("  [OBP]   bytes_remaining    = 0x%02x (%d bytes)\n", header.bytes_remaining, header.bytes_remaining);

    ////////////////////////////////////////////////////////////////////////////
    // payload
    ////////////////////////////////////////////////////////////////////////////

    int payload_size = header.bytes_remaining - (16 + 4); // everything before checksum and footer
    if (payload_size > 0)
        dumpHex("  [OBP] Payload", "  [OBP]   ", msg+44, payload_size);

    if (len < 64)
    {
        printf("ERROR: no checksum or footer? (len = %d)\n", len);

        // dump any bytes beyond header
        if (len > 44)
        {
            printf("Trailing data:");
            for (int p = 44; p < len; p++)
                printf(" 0x%02x", msg[p]);
            printf("\n");
        }

        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    // checksum
    ////////////////////////////////////////////////////////////////////////////

    unsigned char *checksum = msg + sizeof(header) + payload_size;
    printf("  [OBP] Checksum:");
    for (int i = 0; i < 16; i++)
        printf(" 0x%02x", checksum[i]);
    printf("\n");

    ////////////////////////////////////////////////////////////////////////////
    // footer
    ////////////////////////////////////////////////////////////////////////////

    unsigned char *footer = checksum + 16;
    printf("  [OBP] Footer:");
    for (int i = 0; i < 4; i++)
        printf(" 0x%02x", footer[i]);
    printf("\n");

    if (len != 64 + payload_size)
        printf("ERROR: header %zu + payload %d + checksum 16 + footer 4 = %zu (len = %d)\n",
            sizeof(header), payload_size, sizeof(header) + payload_size + 16 + 4, len);
}

// execute an OBP request-response pair over serial (returns 0 on success)
int sendOBPMessage(OBPExchange *xfer)
{
    if (!xfer)
    {
        printf("%s: null OBPExchange\n", __FUNCTION__);
        return 1;
    }

    xfer->actual_response_len = 0;

    if (xfer->request_len > 16)
    {
        printf("%s: currently only supports immediate-mode request params <= 16 bytes\n", __FUNCTION__);
        return 1;
    }

    // check that compiler / architecture hasn't broken anything
    if (sizeof(OBPHeader) != 44)
    {
        printf("%s: header size is %u bytes (expected 44)\n", __FUNCTION__, (unsigned) sizeof(OBPHeader));
        return 1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // define outbound message (header, checksum, footer)
    ////////////////////////////////////////////////////////////////////////////

    // header
    OBPHeader header;
    memset(&header, 0, sizeof(header));
    header.start_bytes[0] = 0xc1;
    header.start_bytes[1] = 0xc0;
    header.protocol_version = 0x1000;
    header.flags = 0x0000; // no ACK requested
    header.message_type = xfer->message_type;
    header.immediate_data_len = xfer->request_len;
    for (unsigned i = 0; i < xfer->request_len; i++)
        header.immediate_data[i] = xfer->request[i];
    header.bytes_remaining = 16 + 4; // i.e. 0x14, 16-byte checksum + 4-byte footer

    // checksum
    unsigned char checksum[16];
    memset(checksum, 0, sizeof(checksum));

    // footer
    unsigned char footer[] = { 0xc5, 0xc4, 0xc3, 0xc2 };

    ////////////////////////////////////////////////////////////////////////////
    // assemble into one message
    ////////////////////////////////////////////////////////////////////////////

    // TODO: merge message with header, checksum, and footer, to avoid this
    //       extra storage and copy
    unsigned char message[64];
    memset(message, 0, sizeof(message));

    memcpy((void*) message + 0,       (void*) &header,  sizeof(header));
    memcpy((void*) message + 44,      (void*) checksum, sizeof(checksum));
    memcpy((void*) message + 44 + 16, (void*) footer,   sizeof(footer));

    ////////////////////////////////////////////////////////////////////////////
    // send message
    ////////////////////////////////////////////////////////////////////////////

    write_buffer(message, sizeof(message));

    ////////////////////////////////////////////////////////////////////////////
    // read response, if one is expected
    ////////////////////////////////////////////////////////////////////////////

    if (xfer->response_len > 0 && xfer->response)
    {
        unsigned expected_response_size = sizeof(message) + xfer->extra_response_len;
        unsigned char *response = (unsigned char*) malloc(expected_response_size);

        int bytes_read = read_buffer(response, expected_response_size);
        if (bytes_read != expected_response_size)
            printf("%s: only read %d of expected %u byte response\n",
                __FUNCTION__, bytes_read, expected_response_size);

        OBPHeader *response_header = (OBPHeader*) response;
        if (0 != response_header->err_no)
        {
            printf("%s: response contained error: %s\n",
                __FUNCTION__, getOBPError(response_header->err_no));
            free(response);
            return -1;
        }

        unsigned bytes_copied = 0;

        // extract response from returned immediate_data
        if (response_header->immediate_data_len > 0)
        {
            unsigned bytes_to_copy = response_header->immediate_data_len;
            if (bytes_to_copy + bytes_copied > xfer->response_len)
            {
                printf("%s: dropped %d immediate_data bytes (insufficient room in output buffer)\n",
                    __FUNCTION__, xfer->response_len - (bytes_to_copy + bytes_copied));
                bytes_to_copy = xfer->response_len - (bytes_to_copy + bytes_copied);
            }
            if (bytes_to_copy > 0)
            {
                memcpy(xfer->response, response_header->immediate_data, bytes_to_copy);
                bytes_copied += bytes_to_copy;
            }
        }

        // extract any remaining_data
        if (bytes_read > sizeof(message) && response_header->bytes_remaining > sizeof(checksum) + sizeof(footer))
        {
            // extract remaining_data from response
            unsigned bytes_to_copy = bytes_read - sizeof(message);
            if (bytes_to_copy + bytes_copied > xfer->response_len)
            {
                printf("%s: dropped %d remaining_data bytes (not enough room in output buffer)\n",
                    __FUNCTION__, xfer->response_len - (bytes_to_copy + bytes_copied));
                bytes_to_copy = xfer->response_len - bytes_copied;
            }

            if (bytes_to_copy > 0)
                memcpy(xfer->response+ bytes_copied, response + sizeof(header), bytes_to_copy);
            bytes_copied += bytes_to_copy;
        }

        xfer->actual_response_len = bytes_copied;

        // TODO: validate checksum
        // TODO: validate footer

        // cleanup
        free(response);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Application
////////////////////////////////////////////////////////////////////////////////

void usage(char **argv)
{
#ifdef RS232
    printf("Usage:\n\n"
           "  %s [--verbose-com] [--verbose-obp] --dev /dev/foo [--info | --partial N | --corrected | --raw | --boxcar N | --integration N]\n\n"
           "Options:\n"
           "  --dev /path/to/device    path to serial driver, e.g. /dev/ttyS0, /dev/tty.usbserial, etc\n"
           "  --verbose-com            debug serial communications\n"
           "  --verbose-obp            debug Ocean Binary Protocol\n"
           "  --info                   read-only info\n"
           "  --partial N              get partial corrected spectra (N=increment amount)\n"
           "  --corrected              get corrected spectra\n"
           "  --raw                    get raw spectra\n"
           "  --boxcar N               set boxcar width (0-15)\n"
           "  --binning N              set pixel binning factor (0-255)\n"
           "  --integration N          set integration time (10+)\n",
           argv[0]);
#else
    printf("Usage:\n\n"
        "  %s [--verbose-com] [--verbose-obp] [--info | --partial | --corrected | --raw | --boxcar N | --integration N]\n\n"
           "Options:\n"
           "  --verbose-com            debug serial communications\n"
           "  --verbose-obp            debug Ocean Binary Protocol\n"
           "  --info                   read-only info\n"
           "  --partial=N              get partial corrected spectra (N=increment amount)\n"
           "  --corrected              get corrected spectra\n"
           "  --raw                    get raw spectra\n"
           "  --boxcar N               set boxcar width (0-15)\n"
           "  --binning N              set pixel binning factor (0-255)\n"
           "  --integration N          set integration time (10+)\n",
           argv[0]);
#endif
    exit(1);
}

void parse_opts(int argc, char **argv)
{
    static const struct option opts[] = {
#ifdef RS232
        { "dev",         required_argument, NULL, 0 },
#endif
        { "verbose-com", no_argument,       NULL, 0 },
        { "verbose-obp", no_argument,       NULL, 0 },
        { "info",        no_argument,       NULL, 0 },
        { "partial",     optional_argument, NULL, 0 },
        { "corrected",   no_argument,       NULL, 0 },
        { "raw",         no_argument,       NULL, 0 },
        { "boxcar",      required_argument, NULL, 0 },
        { "binning",     required_argument, NULL, 0 },
        { "integration", required_argument, NULL, 0 },
        { NULL, no_argument, NULL, 0 }
    };

    char *endptr = NULL;
    int longIndex = 0;
    int opt = getopt_long(argc, argv, "", opts, &longIndex);
    while (opt != -1)
    {
        if (opt == 0)
        {
#ifdef RS232
            if (!strcmp("dev", opts[longIndex].name)) 
                serial_path = optarg;
            else 
#endif
                if (!strcmp("verbose-com", opts[longIndex].name))
                    enable_verbose_com = 1;
                else if (!strcmp("verbose-obp", opts[longIndex].name))
                    enable_verbose_obp = 1;
                else if (!strcmp("integration", opts[longIndex].name)) {
                    opt_integration_time = (unsigned int)strtol(optarg, &endptr, 10);
                    if (opt_integration_time < 10) {
                        opt_integration_time = 0;
                        printf("Invalid integration time, must be >= 10\n");
                        usage(argv);
                    }
                }
                else if (!strcmp("boxcar", opts[longIndex].name)) {
                    opt_boxcar = (int)strtol(optarg, &endptr, 10);
                    if (opt_boxcar < 0 || opt_boxcar > 15) {
                        opt_boxcar = 0;
                        printf("Invalid boxcar size, must be 0-15\n");
                        usage(argv);
                    }
                    else {
                        opt_boxcar += 1;
                    }
                }
                else if (!strcmp("binning", opts[longIndex].name)) {
                    opt_binning = (int)strtol(optarg, &endptr, 10);
                    if (opt_binning < 0 || opt_binning > 255) {
                        opt_binning = 0;
                        printf("Invalid pixel binning factor, must be 0-255\n");
                        usage(argv);
                    }
                    else {
                        opt_binning += 1;
                    }
                }
                else if (!strcmp("info", opts[longIndex].name))
                    opt_info = 1;
                else if (!strcmp("partial", opts[longIndex].name)) {
                    opt_partial = 1;
                    if (optarg) {
                        int temp_partial = (int)strtol(optarg, &endptr, 10);
                        if (temp_partial == 0) {
                            opt_partial = 0;
                            printf("Invalid partial increment, can not be 0\n");
                            usage(argv);
                        }
                        else {
                            opt_partial = temp_partial;
                        }
                    }
                }
                else if (!strcmp("corrected", opts[longIndex].name))
                    opt_corrected = 1;
                else if (!strcmp("raw", opts[longIndex].name))
                    opt_raw = 1;
                else
                    usage(argv);
        }
        else
            usage(argv);

        opt = getopt_long(argc, argv, "", opts, &longIndex);
    }

#ifdef RS232
    // validate only required argument
    if (!serial_path || !strlen(serial_path))
        usage(argv);
#endif
}

int main(int argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////
    // process cmd-line args
    ////////////////////////////////////////////////////////////////////////////

    parse_opts(argc, argv);

    printf("Testing STS/Spark Serial API.\n");

    ////////////////////////////////////////////////////////////////////////////
    // initialization
    ////////////////////////////////////////////////////////////////////////////

#ifdef RS232
    printf("\nopening %s...\n", serial_path);
    serial_fd = open(serial_path, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0)
    {
        printf("error %d opening %s: %s\n", errno, serial_path, strerror(errno));
        exit(1);
    }
    printf("  configuring %s to 8-N-1 at 9600 baud...\n", serial_path);
    set_interface_attribs(serial_fd, B9600, 0);    // set speed to 9600 bps, 8n1 (no parity)
    printf("  unblocking %s...\n", serial_path);
    set_blocking(serial_fd, 0);                    // set no blocking
#else
    char ver[256];
    seabreeze_get_api_version_string(ver, sizeof(ver));
    printf("SeaBreeze API Version: %s\n", ver);
    seabreeze_open_spectrometer(spec_index, &error);
    if (error)
    {
        printf("Error opening spectrometer\n");
        return 1;
    }
    char model[16];
    seabreeze_get_model(spec_index, &error, model, sizeof(model));
    if (error)
    {
        printf("Error getting model\n");
        return 1;
    }
    if (strncmp(model, "STS", 3) && strncmp(model, "SPARK", 5))
    {
        printf("WARNING: This program is only designed to test Ocean Optics STS and Spark spectrometers!\n");
        printf("         Proceed at your own risk...\n\n");
    }
#endif

    ////////////////////////////////////////////////////////////////////////////
    // prepare for OBP exchanges
    ////////////////////////////////////////////////////////////////////////////

    OBPExchange xfer;

    // Note: following blocks are set in if(1/0) to simplify turning individual
    //       tests on or off.


    ////////////////////////////////////////////////////////////////////////////
    // set integration time
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_integration_time)
    {
        unsigned int integration_time = opt_integration_time;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_INTEGRATION_TIME;
        xfer.request_len = sizeof(integration_time);
        xfer.request = (unsigned char *)&integration_time;

        printf("\nsetting integration time...\n");
        if (sendOBPMessage(&xfer)) {
            printf("ERROR: unable to execute SET_INTEGRATION_TIME transaction\n");
            return -1;
        }
        return 0;
    }


    ////////////////////////////////////////////////////////////////////////////
    // set boxcar width
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_boxcar)
    {
        unsigned char boxcar = opt_boxcar - 1;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_BOXCAR_WIDTH;
        xfer.request_len = sizeof(boxcar);
        xfer.request = (unsigned char *)&boxcar;

        printf("\nsetting boxcar width...\n");
        if (sendOBPMessage(&xfer)) {
            printf("ERROR: unable to execute SET_BOXCAR_WIDTH transaction\n");
            return -1;
        }
        return 0;
    }


    ////////////////////////////////////////////////////////////////////////////
    // set pixel binning factor
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_binning)
    {
        unsigned char pixel_binning = opt_binning - 1;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_BINNING_FACTOR;
        xfer.request_len = sizeof(pixel_binning);
        xfer.request = (unsigned char *)&pixel_binning;

        printf("\nsetting pixel binning factor...\n");
        if (sendOBPMessage(&xfer)) {
            printf("ERROR: unable to execute SET_BINNING_FACTOR transaction\n");
            return -1;
        }
        return 0;
    }


    ////////////////////////////////////////////////////////////////////////////
    // get corrected spectrum
    ////////////////////////////////////////////////////////////////////////////

    if (opt_corrected)
    {
        int rtnval = 0;
        unsigned short spectrum[1024];
        int numSpectrum = sizeof(spectrum) / 2;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_CORRECTED_SPECTRUM;
        xfer.response_len = sizeof(spectrum);
        xfer.response = (unsigned char *) spectrum;
        xfer.extra_response_len = sizeof(spectrum); // entire spectrum is payload, none is immediate

        printf("\ngetting corrected spectrum...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == sizeof(spectrum)) {
                printf("Retrieved corrected spectra of %u pixels\n", numSpectrum);
                for (int i = 0; i < numSpectrum; i++) {
                    printf("%d, %u, 0x%04X\n", i, (unsigned int) spectrum[i], (unsigned int) spectrum[i]);
                }
            }
            else {
                printf("ERROR: expected %tu bytes back from get_spectrum, received %u\n",
                    sizeof(spectrum), xfer.actual_response_len);
                rtnval = -1;
            }
        }
        else {
            printf("ERROR: unable to execute GET_CORRECTED_SPECTRUM transaction\n");
            rtnval = -1;
        }

        return rtnval;
    }

    ////////////////////////////////////////////////////////////////////////////
    // get raw spectrum
    ////////////////////////////////////////////////////////////////////////////

    if (opt_raw)
    {
        int rtnval = 0;
        unsigned short spectrum[1024];
        int numSpectrum = sizeof(spectrum) / 2;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_RAW_SPECTRUM;
        xfer.response_len = sizeof(spectrum);
        xfer.response = (unsigned char *)spectrum;
        xfer.extra_response_len = sizeof(spectrum); // entire spectrum is payload, none is immediate

        printf("\ngetting raw spectrum...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == sizeof(spectrum)) {
                printf("Retrieved raw spectra of %u pixels\n", numSpectrum);
                for (int i = 0; i < numSpectrum; i++) {
                    printf("%d, %u, 0x%04X\n", i, (unsigned int)spectrum[i], (unsigned int)spectrum[i]);
                }
            }
            else {
                printf("ERROR: expected %tu bytes back from get_spectrum, received %u\n",
                    sizeof(spectrum), xfer.actual_response_len);
                rtnval = -1;
            }
        }
        else {
            printf("ERROR: unable to execute GET_RAW_SPECTRUM transaction\n");
            rtnval = -1;
        }

        return rtnval;
    }

    ////////////////////////////////////////////////////////////////////////////
    // get partial spectrum
    ////////////////////////////////////////////////////////////////////////////

    if (opt_partial)
    {
        int rtnval = 0;

        // define subspectrum specification
        unsigned short specification[] =
        {
            0x0001,
            (unsigned short) opt_partial
        };

        // send subspectrum specification
        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_SUBSPECTRUM_SPEC;
        xfer.request_len = sizeof(specification);
        xfer.request = (unsigned char *)specification;

        printf("\nspecifying subspectrum, mode=1, increment=%d...\n", opt_partial);
        if (!sendOBPMessage(&xfer)) {
            unsigned short spectrum[1024];
            int numSpectrum = sizeof(spectrum) / 2;
            numSpectrum /= opt_partial;
            int spectrumSize = numSpectrum << 1;

            memset(&xfer, 0, sizeof(xfer));
            xfer.message_type = OBP_MESSAGE_GET_SUBSPECTRUM;
            xfer.response_len = spectrumSize;
            xfer.response = (unsigned char *)spectrum;
            if (numSpectrum > 8) {
                xfer.extra_response_len = spectrumSize; // entire spectrum is payload, none is immediate
            }

            printf("\ngetting partial spectrum, increment=%d...\n", opt_partial);
            if (!sendOBPMessage(&xfer))
            {
                if (xfer.actual_response_len == spectrumSize) {
                    printf("Retrieved raw spectra of %u pixels\n", numSpectrum);
                    for (int i = 0; i < numSpectrum; i++) {
                        printf("%d, %u, 0x%04X\n", i*opt_partial, (unsigned int)spectrum[i], (unsigned int)spectrum[i]);
                    }
                }
                else {
                    printf("ERROR: expected %tu bytes back from get_spectrum, received %u\n",
                        sizeof(spectrum), xfer.actual_response_len);
                    rtnval = -1;
                }
            }
            else {
                printf("ERROR: unable to execute GET_PARTIAL_SPECTRUM transaction\n");
                rtnval = -1;
            }

        }
        else {
            printf("ERROR: unable to execute SET_SUBSPECTRUM_SPECIFICATION transaction\n");
            rtnval = -1;
        }

        return rtnval;
    }

    ////////////////////////////////////////////////////////////////////////////
    // reset defaults
    ////////////////////////////////////////////////////////////////////////////

    if (0)
    {
        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_RESET_DEFAULTS;

        printf("\nresetting defaults...\n");
        if (!sendOBPMessage(&xfer))
            printf("successfully reset defaults\n");
        else
            printf("ERROR: was unable to reset defaults\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get flow-control mode
    ////////////////////////////////////////////////////////////////////////////

    if (0)
    {
        unsigned char mode = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_FLOW_CONTROL;
        xfer.response_len = sizeof(mode);
        xfer.response     = &mode;

        printf("\ngetting flow-control mode...\n");
        if (!sendOBPMessage(&xfer))
            printf("Flow control mode: %d\n", mode);
        else
            printf("ERROR: unable to determine flow control mode\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get hardware revision
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char ver = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_HARDWARE_VERSION;
        xfer.response_len = sizeof(ver);
        xfer.response     = &ver;

        printf("\ngetting hardware revision...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == xfer.response_len)
                printf("Hardware Revision: %02x (%u)\n", ver, ver);
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                    xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_HARDWARE_VERSION transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get firmware revision
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char ver[2];

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type       = OBP_MESSAGE_GET_FIRMWARE_VERSION;
        xfer.response_len       = sizeof(ver);
        xfer.response           = ver;

        printf("\ngetting firmware revision...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == xfer.response_len)
            {
                // convert LSB-MSB
                unsigned short firmware_revision = 0;
                firmware_revision = ver[0] | (ver[1] << 8);
                printf("Firmware Revision: %04x (%u)\n", firmware_revision, firmware_revision);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                    xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_FIRMWARE_VERSION transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get serial number
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char serial_number_len = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_SERIAL_NUMBER_LEN;
        xfer.response_len = sizeof(serial_number_len);
        xfer.response     = &serial_number_len;

        printf("\ngetting serial number length...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("Serial Number Length: %u\n", serial_number_len);

                unsigned char *serial_number = (unsigned char*) malloc(serial_number_len);
                memset(serial_number, 0, serial_number_len);

                memset(&xfer, 0, sizeof(xfer));
                xfer.message_type = OBP_MESSAGE_GET_SERIAL_NUMBER;
                xfer.response_len = serial_number_len;
                xfer.response     = serial_number;

                printf("\ngetting serial number...\n");
                if (!sendOBPMessage(&xfer))
                    printf("Serial Number: %s\n", serial_number);
                else
                    printf("ERROR: error with get_serial_number exchange\n");
                
                free(serial_number);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                    xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_SERIAL_NUMBER_LEN transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get boxcar width
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char boxcar_width = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_BOXCAR_WIDTH;
        xfer.response_len = sizeof(boxcar_width);
        xfer.response = &boxcar_width;

        printf("\ngetting boxcar width...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == xfer.response_len)
                printf("Boxcar Width: %u\n", (unsigned int)boxcar_width);
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_BOXCAR_WIDTH transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get pixel binning factor width
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char binning_factor = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_BINNING_FACTOR;
        xfer.response_len = sizeof(binning_factor);
        xfer.response = &binning_factor;

        printf("\ngetting pixel binning factor...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == xfer.response_len)
                printf("Pixel Binning Factor: %u\n", (unsigned int)binning_factor);
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_BINNING_FACTOR transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // set integration time
    ////////////////////////////////////////////////////////////////////////////

    // STS will not take acquisition unless you first set integration time
    if (1 && !opt_info)
    {
        unsigned char integration_time[] = { 0xa0, 0x86, 0x01, 0x00 }; // 100ms -> 100,000us -> hex -> little-endian

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_INTEGRATION_TIME;
        xfer.request_len  = sizeof(integration_time);
        xfer.request      = integration_time;

        printf("\nsetting integration time...\n");
        if (sendOBPMessage(&xfer))
            printf("ERROR: unable to execute SET_INTEGRATION_TIME transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get corrected spectrum
    ////////////////////////////////////////////////////////////////////////////

    if (1 && !opt_info)
    {
        unsigned char spectrum[2048];

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type       = OBP_MESSAGE_GET_CORRECTED_SPECTRUM;
        xfer.response_len       = sizeof(spectrum);
        xfer.response           = spectrum;
        xfer.extra_response_len = sizeof(spectrum); // entire spectrum is payload, none is immediate

        printf("\ngetting corrected spectrum...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == sizeof(spectrum))
                printf("Retrieved spectra of %zu pixels\n", sizeof(spectrum) / 2);
            else
                printf("ERROR: expected %zu bytes back from get_spectrum, received %u\n",
                    sizeof(spectrum), xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_CORRECTED_SPECTRUM transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // define and get partial corrected spectrum
    ////////////////////////////////////////////////////////////////////////////

    if (1 && !opt_info)
    {
        // define subspectrum specification
        unsigned char specification[] =
        {
            0x02, 0x00, // mode 2 (LSB first)
            0x64, 0x00, // start at pixel 100
            0x01, 0x00, // increment 1 pixel at a time
            0x0a, 0x00  // return 10 pixels
        };

        // send subspectrum specification
        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_SUBSPECTRUM_SPEC;
        xfer.request_len  = sizeof(specification);
        xfer.request      = specification;

        printf("\nspecifying subspectrum...\n");
        if (!sendOBPMessage(&xfer))
        {
            // retrieve the processed subspectrum
            unsigned char subspectrum[20]; // 10 pixels x 16-bit pixels

            memset(&xfer, 0, sizeof(xfer));
            xfer.message_type       = OBP_MESSAGE_GET_SUBSPECTRUM;
            xfer.response_len       = sizeof(subspectrum);
            xfer.response           = subspectrum;
            xfer.extra_response_len = sizeof(subspectrum); // all data is payload, none is immediate

            printf("retrieving subspectrum...\n");
            if (!sendOBPMessage(&xfer))
            {
                if (sizeof(subspectrum) == xfer.actual_response_len)
                {
                    printf("\nPixels 100-109:\n");
                    for (unsigned i = 0; i < 10; i++)
                    {
                        unsigned pixel = 100 + i;
                        unsigned short value = subspectrum[i * 2] | (subspectrum[i * 2 + 1] << 8);
                        printf("  %u (0x%04x): %5u (0x%04x)\n", pixel, pixel, value, value);
                    }
                }
                else
                    printf("ERROR: expected %zu bytes back from get_and_send_partial_corrected_spectrum, received %u\n",
                        sizeof(subspectrum), xfer.actual_response_len);
            }
            else
                printf("ERROR: unable to execute GET_SUBSPECTRUM transaction\n");
        }
        else
            printf("ERROR: unable to execute SET_SUBSPECTRUM_SPECIFICATION transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get wavelength calibration coefficients
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char coeff_count = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type       = OBP_MESSAGE_GET_WL_COEFF_COUNT;
        xfer.response_len       = sizeof(coeff_count);
        xfer.response           = &coeff_count;

        printf("\ngetting wavelength calibration coeff count...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("Wavecal coeff count: %u\n", coeff_count);

                for (unsigned char i = 0; i < coeff_count; i++)
                {
                    unsigned char buf[4];
                    memset(buf, 0, sizeof(buf));

                    memset(&xfer, 0, sizeof(xfer));
                    xfer.message_type = OBP_MESSAGE_GET_WL_COEFF;
                    xfer.request_len  = 1;
                    xfer.request      = &i;
                    xfer.response_len = sizeof(buf);
                    xfer.response     = buf;

                    if (!sendOBPMessage(&xfer))
                    {
                        float* coeff = (float*) buf;
                        printf("  Wavecal coeff %u: %.2e\n", i, *coeff);
                    }
                    else
                        printf("ERROR: error with get_wavecal_coeff(%u) exchange\n", i);
                }
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                    xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_WL_COEFF_COUNT transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get NLC 
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char coeff_count = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type       = OBP_MESSAGE_GET_NLC_COEFF_COUNT;
        xfer.response_len       = sizeof(coeff_count);
        xfer.response           = &coeff_count;

        printf("\ngetting nonlinearity coeff count...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("Nonlinearity coeff count: %u\n", coeff_count);

                for (unsigned char i = 0; i < coeff_count; i++)
                {
                    unsigned char buf[4];
                    memset(buf, 0, sizeof(buf));

                    memset(&xfer, 0, sizeof(xfer));
                    xfer.message_type = OBP_MESSAGE_GET_NLC_COEFF;
                    xfer.request_len  = 1;
                    xfer.request      = &i;
                    xfer.response_len = sizeof(buf);
                    xfer.response     = buf;

                    if (!sendOBPMessage(&xfer))
                    {
                        float* coeff = (float*) buf;
                        printf("  NLC coeff %u: %.2e\n", i, *coeff);
                    }
                    else
                        printf("ERROR: error with get_nlc_coeff(%u) exchange\n", i);
                }
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                    xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_NLC_COEFF_COUNT transaction\n");
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // user strings
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char user_string_count = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type       = OBP_MESSAGE_GET_USER_STRING_COUNT;
        xfer.response_len       = sizeof(user_string_count);
        xfer.response           = &user_string_count;

        printf("\ngetting user string count...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("user string count: %u\n", user_string_count);

                unsigned short user_string_len = 0;

                memset(&xfer, 0, sizeof(xfer));
                xfer.message_type       = OBP_MESSAGE_GET_USER_STRING_LEN;
                xfer.response_len       = sizeof(user_string_len);
                xfer.response           = (unsigned char*) &user_string_len;

                printf("\ngetting user string len...\n");
                if (!sendOBPMessage(&xfer))
                {
                    if (xfer.response_len == xfer.actual_response_len)
                    {
                        printf("user string length: %u\n", user_string_len);

                        for (unsigned char i = 0; i < user_string_count; i++)
                        {
                            unsigned char buf[user_string_len];
                            memset(buf, 0, sizeof(buf));

                            memset(&xfer, 0, sizeof(xfer));
                            xfer.message_type = OBP_MESSAGE_GET_USER_STRING;;
                            xfer.request_len  = 1;
                            xfer.request      = &i;
                            xfer.response_len = sizeof(buf);
                            xfer.response     = buf;

                            if (!sendOBPMessage(&xfer))
                                printf("  User string %02u: [%s]\n", i, buf);
                            else
                                printf("ERROR: error with get_user_string(%u) exchange\n", i);
                        }
                    }
                    else
                        printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                            xfer.response_len, xfer.message_type, xfer.actual_response_len);
                }
                else
                    printf("ERROR: unable to execute GET_USER_STRING_LEN transaction\n");
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                    xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_USER_STRING_COUNT transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // cleanup
    ////////////////////////////////////////////////////////////////////////////

#ifdef RS232
    int close_result = close(serial_fd);
    if (close_result != 0)
        printf("error %d from close(%d)\n", errno, serial_fd);
#else
    seabreeze_close_spectrometer(spec_index, &error);
    if (error)
        printf("error %d from seabreeze_close_spectrometer\n", errno);
#endif

    return 0;
}
