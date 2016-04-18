/**
* @file     demo-qepro-api.c
* @author   Mark Zieg <mark.zieg@oceanoptics.com>
* @author   Doug Anderson <doug.anderson@oceanoptics.com>
* @date     2015-02-06
*
* Demonstrate how to communicate via the QEPro / Ocean Binary Protocol (OBP)
* API, using either USB (via SeaBreeze) or raw serial comms.
*
* Copied and modified from: demo-sts-api.c
*
* Serial functions borrowed from:
* http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
*
* Please see QEPro Data Sheet for Ocean Binary Protocol API:
* http://oceanoptics.com/support/technical-documents/oem-data-sheet-qepro/
*
* LICENSE:
*
* SeaBreeze Copyright (C) 2015, Ocean Optics Inc
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

// *** UNCOMMENT THIS LINE to use RS232 rather than SeaBreeze ***
//#define RS232

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include <termios.h>  // Purposely outside #define RS232 so baud rate can be set via USB
#ifdef RS232
#include <fcntl.h>
#else
#include "api/SeaBreezeWrapper.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Preprocessor Macros
////////////////////////////////////////////////////////////////////////////////

// just a few of the many commands available within OBP
#define OBP_MESSAGE_RESET_DEFAULTS          0x00000001L
#define OBP_MESSAGE_GET_BAUD_RATE           0x00000800L
#define OBP_MESSAGE_SET_BAUD_RATE           0x00000810L
#define OBP_MESSAGE_GET_FLOW_CONTROL        0x00000804L
#define OBP_MESSAGE_SET_FLOW_CONTROL        0x00000814L
#define OBP_MESSAGE_GET_HARDWARE_VERSION    0x00000080L
#define OBP_MESSAGE_GET_FIRMWARE_VERSION    0x00000090L
#define OBP_MESSAGE_GET_SERIAL_NUMBER       0x00000100L
#define OBP_MESSAGE_GET_SERIAL_NUMBER_LEN   0x00000101L
#define OBP_MESSAGE_ABORT_ACQUISITION       0x00100000L

// temperature sensor commands
#define OBP_MESSAGE_GET_TEMP_SENSOR_COUNT   0x00400000L
#define OBP_MESSAGE_READ_ALL_TEMP_SENSORS   0x00400002L

// TEC commands
#define OBP_MESSAGE_GET_TEC_ENABLED         0x00420000L
#define OBP_MESSAGE_GET_TEC_STABLE          0x00420003L
#define OBP_MESSAGE_GET_TEC_SETPOINT        0x00420001L
#define OBP_MESSAGE_GET_TEC_TEMPERATURE     0x00420004L

// Integration time
#define OBP_MESSAGE_GET_INTEGRATION_TIME    0x00110000L
#define OBP_MESSAGE_SET_INTEGRATION_TIME    0x00110010L

// Buffer Specific Commands
#define OBP_MESSAGE_GET_DEVICE_IS_IDLE      0x00100908L
#define OBP_MESSAGE_GET_MAX_BUFFER_SIZE     0x00100820L
#define OBP_MESSAGE_GET_BUFFER_SIZE         0x00100822L
#define OBP_MESSAGE_GET_NUM_IN_BUFFER       0x00100900L
#define OBP_MESSAGE_SET_BUFFER_SIZE         0x00100832L
#define OBP_MESSAGE_GET_BUFFERED_SPECTRA    0x00100928L
#define OBP_MESSAGE_CLEAR_BUFFERS           0x00100830L
#define OBP_MESSAGE_START_BUFFERING         0x00100902L  // Acquire into Buffer (start buffer spectra in accordance with trigger mode)

// TODO: replace with buffered commands
#define OBP_MESSAGE_GET_CORRECTED_SPECTRUM  0x00101000L
#define OBP_MESSAGE_SET_SUBSPECTRUM_SPEC    0x00102010L
#define OBP_MESSAGE_GET_SUBSPECTRUM         0x00102080L

// wavelength coefficients
#define OBP_MESSAGE_GET_WL_COEFF_COUNT      0x00180100L
#define OBP_MESSAGE_GET_WL_COEFF            0x00180101L
#define OBP_MESSAGE_SET_WL_COEFF            0x00180111L

// non-linearity corefficients
#define OBP_MESSAGE_GET_NLC_COEFF_COUNT     0x00181100L
#define OBP_MESSAGE_GET_NLC_COEFF           0x00181101L
#define OBP_MESSAGE_SET_NLC_COEFF           0x00181101L

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

// Taken directly from page 22 of the QEPro Data Sheet
typedef struct OBPHeader_s
{                                       // byte offset
    unsigned char  start_bytes[2];      // 0-1: 0xC1, 0xC0
    unsigned short protocol_version;    // 2-3: 0x1100
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

// Based on pages 11 & 12 in the QEPro Data Sheet
#ifdef _MSC_VER
#pragma pack (push, 4)  // necessary to align "integration_time" after the 8 byte tick count field
#endif
typedef struct OBP_Metadata_s
{                                       // byte offset
    //-- metadata --
    unsigned int       spec_count;          // 0-3
    unsigned long long tick_count __attribute__((packed)); // 4-11 (packed to enforce integration_time alignment)
    unsigned int       integration_time ;    // 12-15
    unsigned short     reserved1;           // 16-17
    unsigned char      trigger_mode;        // 18
    unsigned char      reserved2[13];       // 19-31
} OBP_Metadata; // 32 bytes total
#ifdef _MSC_VER
#pragma pack (pop)
#endif

// Based on pages 11 & 12 in the QEPro Data Sheet
typedef struct OBP_Spectra_s
{                                       // byte offset
    //-- spectrum --
    unsigned int		dummyStart[4];
    unsigned int		darkStart[6];
    unsigned int		spectrum[1024];
    unsigned int		darkEnd[6];
    unsigned int		dummyEnd[4];
} OBP_Spectra; // XXX bytes total

// Based on pages 11 & 12 in the QEPro Data Sheet
typedef struct OBP_SpectraWithMetadata_s
{                                       // byte offset
    OBP_Metadata		metadata;
    OBP_Spectra			spectra;
} OBP_SpectraWithMetadata; // XXX bytes total


////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

int enable_verbose_com = 0;             // debug serial comms to stdout
int enable_verbose_obp = 0;             // debug protocol internals to stdout
unsigned int opt_integration_time = 0;  // integration time to set
unsigned int opt_buffer_size = 0;       // buffer size
int opt_clear = 0;                      // clear all buffered spectra
int opt_abort = 0;                      // abort acquisition and enter idle mode
int opt_acquire = 0;                    // acquire into buffer
int opt_spectra = 0;                    // retrieve spectra with metadata
int opt_baudrate = 0;                   // set the RS-232 baud rate
int opt_flowcontrol = 0;                // set h/w flow control
int opt_reset = 0;                      // reset defaults

#ifdef RS232
const int STS_BAUD_RATE = 9600;  // any of 9600, 19200, 38400, 57600, 115,200, 230400 (AFTER configuring QEPro accordingly)
const int HW_FLOW_CONTROL = 0;     // 0 or 1, must match QEPro setting (doesn't work on all systems... ie Windows)

char *serial_path = NULL;               // "/dev/ttyS0" or similar
int serial_fd = -1;                     // result of open()
#else
const unsigned char STS_REQUEST_ENDPOINT  = 0x01;
const unsigned char STS_RESPONSE_ENDPOINT = 0x81;
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

// Purposely outside #define RS232 so baud rate can be set via USB
int baudrate_to_termspeed(int baudrate) {
    int term_speed = 0;

    switch (baudrate) {
    case 9600:
        term_speed = B9600;
        break;
    case 19200:
        term_speed = B19200;
        break;
    case 38400:
        term_speed = B38400;
        break;
    case 57600:
        term_speed = B57600;
        break;
    case 115200:
        term_speed = B115200;
        break;
    case 230400:
        term_speed = B230400;
        break;
    }

    return term_speed;
}

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
    tty.c_cc[VMIN] = 0;        // read doesn't block
    tty.c_cc[VTIME] = 5;        // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);       // ignore modem ctrl, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |=  parity;
    tty.c_cflag &= ~CSTOPB;
    if (HW_FLOW_CONTROL)
        tty.c_cflag |= CRTSCTS;  // hw flow control - on
    else
        tty.c_cflag &= ~CRTSCTS; // hw flow control - off

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

// computers run faster than the baud, so wait for comms or FIFOs may overflow
void waitForTransmission(int bytesToTransfer)
{
    const double MICROSEC_PER_BIT = 1000000L / STS_BAUD_RATE;

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

// send a buffer to the STS
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
    seabreeze_write_usb(spec_index, &error, STS_REQUEST_ENDPOINT, request, len);
#endif
}

// read a buffer from the STS
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
    total_bytes_read = seabreeze_read_usb(spec_index, &error, STS_RESPONSE_ENDPOINT, response, len);
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
    header.protocol_version = 0x1100;
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
            printf("%s: QEPro response contained error: %s\n",
                __FUNCTION__, getOBPError(response_header->err_no));
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
           "  %s [--verbose-com] [--verbose-obp] --dev /dev/foo [--integration n | --buffersize n | --clear | --abort | --acquire | --spectra | --reset]\n\n"
           "Options:\n"
           "  --dev /path/to/device    path to serial driver, e.g. /dev/ttyS0, /dev/tty.usbserial, etc\n"
           "  --verbose-com            debug serial communications\n"
           "  --verbose-obp            debug Ocean Binary Protocol\n"
           "  --integration            set integration time (uSec >= 8000)\n"
           "  --buffersize n           set max # spectra to be buffered\n"
           "  --clear                  clear all buffered spectra\n"
           "  --abort                  abort acquisition and enter idle mode\n"
           "  --acquire                acquire spectra into buffer\n"
           "  --spectra                get buffered spectra with metadata\n",
           argv[0]);
#else
    printf("Usage:\n\n"
           "  %s [--verbose-com] [--verbose-obp] [--integration n | --buffersize n | --clear | --abort | --acquire | --spectra | --reset]]\n\n"
           "Options:\n"
           "  --verbose-com            debug serial communications\n"
           "  --verbose-obp            debug Ocean Binary Protocol\n"
           "  --integration n          set integration time (uSec >= 8000)\n"
           "  --buffersize n           set max # spectra to be buffered\n"
           "  --clear                  clear all buffered spectra\n"
           "  --abort                  abort acquisition and enter idle mode\n"
           "  --acquire                acquire spectra into buffer\n"
           "  --spectra                get buffered spectra with metadata\n",
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
        { "integration", required_argument, NULL, 0 },
        { "buffersize",  required_argument, NULL, 0 },
        { "clear",       no_argument,       NULL, 0 },
        { "abort",       no_argument,       NULL, 0 },
        { "acquire",     no_argument,       NULL, 0 },
        { "spectra",     no_argument,       NULL, 0 },
        { "reset", no_argument, NULL, 0 },
        { "baudrate", required_argument, NULL, 0 },
        { "flowcontrol", required_argument, NULL, 0 },
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
                if (opt_integration_time < 8000) {
                    opt_integration_time = 0;
                    printf("Invalid integration time, must be >= 8000\n");
                    usage(argv);
                }
            }
            else if (!strcmp("buffersize", opts[longIndex].name)) {
                opt_buffer_size = (unsigned int) strtol(optarg, &endptr, 10);
                if (opt_buffer_size < 1) {
                    opt_buffer_size = 0;
                    printf("Invalid buffer size, must be >= 1\n");
                    usage(argv);
                }
            }
            else if (!strcmp("baudrate", opts[longIndex].name)) {
                opt_baudrate = (unsigned int)strtol(optarg, &endptr, 10);
                if (!baudrate_to_termspeed(opt_baudrate)) {
                    opt_baudrate = 0;
                    printf("Invalid baud rate, must be >= 9600\n");
                    usage(argv);
                }
            }
            else if (!strcmp("flowcontrol", opts[longIndex].name)) {
                opt_flowcontrol = (unsigned int)strtol(optarg, &endptr, 10);
                if (opt_flowcontrol != 0 && opt_flowcontrol != 1) {
                    opt_flowcontrol = 0;
                    printf("Invalid flow control, must be 0 or 1\n");
                    usage(argv);
                }
                else {
                    opt_flowcontrol += 1;
                }
            }
            else if (!strcmp("clear", opts[longIndex].name))
                opt_clear = 1;
            else if (!strcmp("abort", opts[longIndex].name))
                opt_abort = 1;
            else if (!strcmp("acquire", opts[longIndex].name))
                opt_acquire = 1;
            else if (!strcmp("spectra", opts[longIndex].name))
                opt_spectra = 1;
            else if (!strcmp("reset", opts[longIndex].name))
                opt_reset = 1;
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

    printf("Testing QEPro Serial API.\n");

    ////////////////////////////////////////////////////////////////////////////
    // initialization
    ////////////////////////////////////////////////////////////////////////////

#ifdef RS232
    int TERMIOS_SPEED = baudrate_to_termspeed(STS_BAUD_RATE);

    printf("\nopening %s...\n", serial_path);
    serial_fd = open(serial_path, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0)
    {
        printf("error %d opening %s: %s\n", errno, serial_path, strerror(errno));
        exit(1);
    }
    printf("  configuring %s to 8-N-1 at %d baud (%d)...\n", serial_path, STS_BAUD_RATE, TERMIOS_SPEED);
    set_interface_attribs(serial_fd, TERMIOS_SPEED, 0);    // set speed to STS_BAUD_RATE bps, 8n1 (no parity)
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
    if (strncmp(model, "QE-Pro", 3))
    {
        printf("WARNING: This program is only designed to test Ocean Optics QEPro spectrometers!\n");
        printf("         Proceed at your own risk...\n\n");
    }
#endif

    #if (0)
        printf("sizeof(short)=%tu\n", sizeof(short));
        printf("sizeof(int)=%tu\n", sizeof(int));
        printf("sizeof(long)=%tu\n", sizeof(long));
        printf("sizeof(long long)=%tu\n", sizeof(long long));
    #endif

    if (sizeof(OBPHeader) != 44) {
        printf("WARNING: sizeof(OBPHeader_s)=%tu, should be 44\n", sizeof(OBPHeader));
    }
    if (sizeof(OBP_Metadata) != 32) {
        printf("WARNING: sizeof(OBP_Metadata_s)=%tu, should be 32\n", sizeof(OBP_Metadata));
    }
    if (sizeof(OBP_Spectra) != 4176) {
        printf("WARNING: sizeof(OBP_Spectra_s)=%tu, should be 4176\n", sizeof(OBP_Spectra));
    }
    if (sizeof(OBP_SpectraWithMetadata) != 4208) {
        printf("WARNING: sizeof(OBP_SpectraWithMetadata_s)=%tu, should be 4208\n", sizeof(OBP_SpectraWithMetadata));
    }


    ////////////////////////////////////////////////////////////////////////////
    // prepare for OBP exchanges
    ////////////////////////////////////////////////////////////////////////////

    OBPExchange xfer;

    // Note: following blocks are set in if(1/0) to simplify turning individual
    //       tests on or off.

    //*************************************************************************
    //*************************************************************************
    //
    // Command Line Options
    //
    //*************************************************************************
    //*************************************************************************

    ////////////////////////////////////////////////////////////////////////////
    // set integration time
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_integration_time)
    {
        unsigned int integration_time = opt_integration_time;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_INTEGRATION_TIME;
        xfer.request_len = sizeof(integration_time);
        xfer.request = (unsigned char *) &integration_time;

        printf("\nsetting integration time...\n");
        if (sendOBPMessage(&xfer)) {
            printf("ERROR: unable to execute SET_INTEGRATION_TIME transaction\n");
            return -1;
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // set buffer size
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_buffer_size)
    {
        unsigned int buffer_size = opt_buffer_size;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_BUFFER_SIZE;
        xfer.request_len = sizeof(buffer_size);
        xfer.request = (unsigned char *)&buffer_size;

        printf("\nsetting buffer size...\n");
        if (sendOBPMessage(&xfer)) {
            printf("ERROR: unable to execute SET_BUFFER_SIZE transaction\n");
            return -1;
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // abort acquisition
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_abort)
    {
        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_ABORT_ACQUISITION;

        printf("\naborting acqusition...\n");
        if (!sendOBPMessage(&xfer)) {
            printf("  abort successful\n");
        }
        else {
            printf("ERROR: unable to execute ABORT_ACQUISITION transaction\n");
            return -1;
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // acquire acquisition
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_acquire)
    {
        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_START_BUFFERING;

        printf("\nstarting spectra buffering...\n");
        if (!sendOBPMessage(&xfer)) {
            printf("  acquire start successful\n");
        }
        else {
            printf("ERROR: unable to execute START_BUFFERING transaction\n");
            return -1;
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // clear all spectra from buffers
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_clear)
    {
        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_CLEAR_BUFFERS;

        printf("\nclearing buffered spectra...\n");
        if (!sendOBPMessage(&xfer)) {
            printf("  buffered spectra successfully cleared\n");
        }
        else {
            printf("ERROR: unable to execute CLEAR_BUFFERS transaction\n");
            return -1;
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // read spectra with metadata
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_spectra)
    {
        int rtn = 0;

        OBP_SpectraWithMetadata *pBufferedSpectra = (OBP_SpectraWithMetadata *)malloc(sizeof(OBP_SpectraWithMetadata));

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_BUFFERED_SPECTRA;
        xfer.response = (unsigned char *)pBufferedSpectra;
        xfer.response_len = sizeof(OBP_SpectraWithMetadata);
        xfer.extra_response_len = sizeof(OBP_SpectraWithMetadata);

        printf("\ngetting buffered spectra with metadata...\n");
        if (!sendOBPMessage(&xfer)) {
            printf("  Spectrum Count: %u\n", pBufferedSpectra->metadata.spec_count);
            printf("      Tick Count: %llu\n", pBufferedSpectra->metadata.tick_count);
            printf("Integration Time: %u\n", pBufferedSpectra->metadata.integration_time);
            printf("    Trigger Mode: %u\n", (unsigned int)pBufferedSpectra->metadata.trigger_mode);

            printf("\nDummy Start:\n");
            unsigned int *pSpectra = pBufferedSpectra->spectra.dummyStart;
            printf("%08X %08X %08X %08X\n", pSpectra[0], pSpectra[1], pSpectra[2], pSpectra[3]);

            printf("\nDark Start:\n");
            pSpectra = pBufferedSpectra->spectra.darkStart;
            printf("%08X %08X %08X %08X %08X %08X\n", pSpectra[0], pSpectra[1], pSpectra[2],
                pSpectra[3], pSpectra[4], pSpectra[5]);

            printf("\nSpectrum:\n");
            pSpectra = pBufferedSpectra->spectra.spectrum;
            for (int i = 0; i < 2014; i += 8) {
                printf("%08X %08X %08X %08X %08X %08X %08X %08X\n",
                    pSpectra[0], pSpectra[1], pSpectra[2], pSpectra[3],
                    pSpectra[4], pSpectra[5], pSpectra[6], pSpectra[7]);
            }

            printf("\nDark End:\n");
            pSpectra = pBufferedSpectra->spectra.darkEnd;
            printf("%08X %08X %08X %08X %08X %08X\n", pSpectra[0], pSpectra[1], pSpectra[2],
                pSpectra[3], pSpectra[4], pSpectra[5]);

            printf("\nDummy End:\n");
            pSpectra = pBufferedSpectra->spectra.dummyEnd;
            printf("%08X %08X %08X %08X\n", pSpectra[0], pSpectra[1], pSpectra[2], pSpectra[3]);

        }
        else {
            printf("ERROR: unable to execute GET_BUFFERED_SPECTRA transaction\n");
            rtn = -1;
        }

        if (pBufferedSpectra != NULL) {
            free(pBufferedSpectra);
            pBufferedSpectra = NULL;
        }

        return rtn;
    }

    ////////////////////////////////////////////////////////////////////////////
    // set serial baud rate
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_baudrate)
    {
        unsigned int baud_rate = opt_baudrate;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_BAUD_RATE;
        xfer.request_len = sizeof(baud_rate);
        xfer.request = (unsigned char *)&baud_rate;

        printf("\nsetting baud rate...\n");
        if (!sendOBPMessage(&xfer)) {
            printf("baud rate successfully set\n");
        }
        else {
            printf("ERROR: unable to execute SET_BAUD_RATE transaction\n");
            return -1;
        }
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // set serial flow control mode
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_flowcontrol)
    {
        unsigned char flow_control = opt_flowcontrol - 1;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_SET_FLOW_CONTROL;
        xfer.request_len = sizeof(flow_control);
        xfer.request = (unsigned char *)&flow_control;

        printf("\nsetting flow control...\n");
        if (!sendOBPMessage(&xfer)) {
            printf("flow control successfully set\n");
        }
        else {
            printf("ERROR: unable to execute SET_FLOW_CONTROL transaction\n");
            return -1;
        }
        return 0;
    }

    //*************************************************************************
    //*************************************************************************
    //
    // Default Behavior (no command line options except --verbose-xxx)
    //
    //*************************************************************************
    //*************************************************************************


    ////////////////////////////////////////////////////////////////////////////
    // reset defaults
    ////////////////////////////////////////////////////////////////////////////

    if (1 && opt_reset)
    {
        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_RESET_DEFAULTS;

        printf("\nresetting defaults...\n");
        if (!sendOBPMessage(&xfer)) {
            printf("successfully reset defaults\n");
            return 0;
        }
        else {
            printf("ERROR: was unable to reset defaults\n");
            return -1;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // get flow-control mode
    ////////////////////////////////////////////////////////////////////////////

    if (1)
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
    // get RS-232 baud rate
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned int baud_rate = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_BAUD_RATE;
        xfer.response_len = sizeof(baud_rate);
        xfer.response = (unsigned char *)&baud_rate;

        printf("\ngetting RS-232 baud rate...\n");
        if (!sendOBPMessage(&xfer))
            printf("RS-232 baud rate: %d\n", baud_rate);
        else
            printf("ERROR: unable to determine RS-232 baud rate\n");
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
                serial_number = NULL;
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                    xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_SERIAL_NUMBER_LEN transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // get wavecal
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char coeff_count = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type       = OBP_MESSAGE_GET_WL_COEFF_COUNT;
        xfer.response_len       = sizeof(coeff_count);
        xfer.response           = &coeff_count;

        printf("\ngetting wavecal coeff count...\n");
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
    // read Temperature Sensors 
    ////////////////////////////////////////////////////////////////////////////

    const char * QEPRO_TEMP_SENSOR_NAME[] = {
        "CPU",
        "reserved",
        "PCB",
        "TEC thermistor",
        "unexpected",
        "unexpected",
        "unexpected",
        "unexpected",
    };

    if (1)
    {
        unsigned char sensor_count = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_TEMP_SENSOR_COUNT;
        xfer.response_len = sizeof(sensor_count);
        xfer.response = &sensor_count;

        printf("\ngetting temperature sensor count...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("Temperature sensor count: %u\n", sensor_count);

                unsigned int temp_sensor_len = sensor_count*sizeof(float);
                float *temp_sensors = (float*)malloc(temp_sensor_len);
                memset(temp_sensors, 0, temp_sensor_len);

                memset(&xfer, 0, sizeof(xfer));
                xfer.message_type = OBP_MESSAGE_READ_ALL_TEMP_SENSORS;
                xfer.response_len = temp_sensor_len;
                xfer.response = (unsigned char *)temp_sensors;

                printf("\nreading temperature sensors...\n");
                if (!sendOBPMessage(&xfer)) {
                    for (unsigned char i = 0; i < sensor_count; i++) {
                        printf("  Temp Sensor%u: %.2f (%s)\n", i, temp_sensors[i], QEPRO_TEMP_SENSOR_NAME[i]);
                    }
                }
                else
                    printf("ERROR: error with read_all_temp_sensors\n");

                free(temp_sensors);
                temp_sensors = NULL;
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_TEMP_SENSOR_COUNT transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // TEC enabled 
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        printf("\nTEC:\n");

        unsigned char tec_enabled = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_TEC_ENABLED;
        xfer.response_len = sizeof(tec_enabled);
        xfer.response = &tec_enabled;

        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                if (tec_enabled)
                    printf("    Enabled: yes\n");
                else
                    printf("    Enabled: no\n");
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_TEC_ENABLED transaction\n");

        //-----

        unsigned char tec_stable = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_TEC_STABLE;
        xfer.response_len = sizeof(tec_stable);
        xfer.response = &tec_stable;

        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                if (tec_stable)
                    printf("     Stable: yes\n");
                else
                    printf("     Stable: no\n");
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_TEC_STABLE transaction\n");

        //-----

        float tec_setpoint = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_TEC_SETPOINT;
        xfer.response_len = sizeof(tec_setpoint);
        xfer.response = (unsigned char *) &tec_setpoint;

        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("  Set Point: %.2f\n", tec_setpoint);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_TEC_SETPOINT transaction\n");

        //-----

        float tec_temperature = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_TEC_TEMPERATURE;
        xfer.response_len = sizeof(tec_temperature);
        xfer.response = (unsigned char *)&tec_temperature;

        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("       Temp: %.2f\n", tec_temperature);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_TEC_TEMPERATURE transaction\n");

    }

    ////////////////////////////////////////////////////////////////////////////
    // get integration time
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned int integration_time = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_INTEGRATION_TIME;
        xfer.response_len = sizeof(integration_time);
        xfer.response = (unsigned char *)&integration_time;

        printf("\ngetting integration time...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("  Integration Time: %u uSec (0x%08X)\n", integration_time, integration_time);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_INTEGRATION_TIME transaction\n");

    }

    ////////////////////////////////////////////////////////////////////////////
    // get device is idle
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned char device_is_idle = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_DEVICE_IS_IDLE;
        xfer.response_len = sizeof(device_is_idle);
        xfer.response = (unsigned char *)&device_is_idle;

        printf("\nchecking if device is idle...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                if (device_is_idle)
                    printf("  Idle: yes\n");
                else
                    printf("  Idle: no\n");
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_DEVICE_IS_IDLE transaction\n");

    }

    ////////////////////////////////////////////////////////////////////////////
    // get max buffer size
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned int max_buffer_size = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_MAX_BUFFER_SIZE;
        xfer.response_len = sizeof(max_buffer_size);
        xfer.response = (unsigned char *)&max_buffer_size;

        printf("\ngetting max buffer size...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("  Max Buffer: %u (0x%08X)\n", max_buffer_size, max_buffer_size);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_MAX_BUFFER_SIZE transaction\n");

    }

    ////////////////////////////////////////////////////////////////////////////
    // get current buffer size
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned int cur_buffer_size = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_BUFFER_SIZE;
        xfer.response_len = sizeof(cur_buffer_size);
        xfer.response = (unsigned char *)&cur_buffer_size;

        printf("\ngetting current buffer size...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("  Buffer Size: %u (0x%08X)\n", cur_buffer_size, cur_buffer_size);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_BUFFER_SIZE transaction\n");

    }

    ////////////////////////////////////////////////////////////////////////////
    // get num spectra in buffer
    ////////////////////////////////////////////////////////////////////////////

    if (1)
    {
        unsigned int num_in_buffer = 0;

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_NUM_IN_BUFFER;
        xfer.response_len = sizeof(num_in_buffer);
        xfer.response = (unsigned char *)&num_in_buffer;

        printf("\ngetting number of spectra in buffer...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.response_len == xfer.actual_response_len)
            {
                printf("  Num Spectra in Buffer: %u (0x%08X)\n", num_in_buffer, num_in_buffer);
            }
            else
                printf("ERROR: expected %u bytes back from 0x%08x, received %u\n",
                xfer.response_len, xfer.message_type, xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_NUM_IN_BUFFER transaction\n");

    }

    ////////////////////////////////////////////////////////////////////////////
    // get corrected spectrum
    ////////////////////////////////////////////////////////////////////////////

    if (0)
    {
        unsigned char spectrum[2048];

        memset(&xfer, 0, sizeof(xfer));
        xfer.message_type = OBP_MESSAGE_GET_CORRECTED_SPECTRUM;
        xfer.response_len = sizeof(spectrum);
        xfer.response = spectrum;
        xfer.extra_response_len = sizeof(spectrum); // entire spectrum is payload, none is immediate

        printf("\ngetting corrected spectrum...\n");
        if (!sendOBPMessage(&xfer))
        {
            if (xfer.actual_response_len == sizeof(spectrum))
                printf("Retrieved spectra of %tu pixels\n", sizeof(spectrum) / 2);
            else
                printf("ERROR: expected %tu bytes back from get_spectrum, received %u\n",
                sizeof(spectrum), xfer.actual_response_len);
        }
        else
            printf("ERROR: unable to execute GET_CORRECTED_SPECTRUM transaction\n");
    }

    ////////////////////////////////////////////////////////////////////////////
    // define and get partial corrected spectrum
    ////////////////////////////////////////////////////////////////////////////

    if (0)
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
        xfer.request_len = sizeof(specification);
        xfer.request = specification;

        printf("\nspecifying subspectrum...\n");
        if (!sendOBPMessage(&xfer))
        {
            // retrieve the processed subspectrum
            unsigned char subspectrum[20]; // 10 pixels x 16-bit pixels

            memset(&xfer, 0, sizeof(xfer));
            xfer.message_type = OBP_MESSAGE_GET_SUBSPECTRUM;
            xfer.response_len = sizeof(subspectrum);
            xfer.response = subspectrum;
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
                    printf("ERROR: expected %tu bytes back from get_and_send_partial_corrected_spectrum, received %u\n",
                    sizeof(subspectrum), xfer.actual_response_len);
            }
            else
                printf("ERROR: unable to execute GET_SUBSPECTRUM transaction\n");
        }
        else
            printf("ERROR: unable to execute SET_SUBSPECTRUM_SPECIFICATION transaction\n");
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
