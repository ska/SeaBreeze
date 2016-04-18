/**
* @file     demo-strobe-sts.c
* @author   Mark Zieg <mark.zieg@oceanoptics.com>
* @date     2012-11-07
*
* Demonstrate how to set the lamp and strobe period on an STS via raw USB.
*
* In particular, the current version of this code has been tested on an STS-VIS
* with hardware revision 0x03 and firmware revision 0x0036.  Other revisions
* may support or require a different sequence or selection of operations in
* enableLampSTS().
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#include "api/SeaBreezeWrapper.h"

// global for convenience
const int spec_index = 0;
const int strobe_index = 0;
int error = 0;

struct OBPHeader
{
    unsigned char  start_bytes[2];          // 0-1: 0xC1, 0xC0
    unsigned short protocol_version;        // 2-3: 0x1000
    unsigned short flags;                   // 4-5
    unsigned short errno;                   // 6-7
    unsigned int   message_type;            // 8-11
    unsigned int   regarding;               // 12-15
    unsigned char  reserved[6];             // 16-21
    unsigned char  checksum_type;           // 22
    unsigned char  immediate_data_length;   // 23
    unsigned char  immediate_data[16];      // 24-39
    unsigned int   bytes_remaining;         // 40-43
};                                          // 44 bytes total

// returns 0 on success
int sendOBPMessage(unsigned int message_type, unsigned int immediate_data_length,
        unsigned char *immediate_data)
{
    const unsigned char STS_REQUEST_ENDPOINT  = 0x01;
    const unsigned char STS_RESPONSE_ENDPOINT = 0x81;

    int requestAck = 1;

    if (immediate_data_length > 16)
    {
        fprintf(stderr, "ERROR: sendOBPMessage only supports immediate data <= 16 bytes\n");
        error = 1;
        return 1;
    }

    if (sizeof(struct OBPHeader) != 44)
    {
        fprintf(stderr, "ERROR: header size is %u bytes (expected 44)\n", (unsigned) sizeof(struct OBPHeader));
        return 1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // define constituent parts (header, checksum, footer)
    ////////////////////////////////////////////////////////////////////////////

    // header
    struct OBPHeader header;
    memset(&header, 0, sizeof(header));
    header.start_bytes[0] = 0xc1;
    header.start_bytes[1] = 0xc0;
    header.protocol_version = 0x1000;
    header.flags = requestAck ? 0x0004 : 0;
    header.message_type = message_type;
    header.immediate_data_length = immediate_data_length;
    for (unsigned i = 0; i < immediate_data_length; i++)
        header.immediate_data[i] = immediate_data[i];
    header.bytes_remaining = 16 + 4; // i.e. 0x14, 16-byte checksum + 4-byte footer

    // checksum
    unsigned char checksum[16];
    memset(checksum, 0, sizeof(checksum));

    // footer
    unsigned char footer[] = { 0xc5, 0xc4, 0xc3, 0xc2 };

    ////////////////////////////////////////////////////////////////////////////
    // assemble into one message
    ////////////////////////////////////////////////////////////////////////////

    unsigned char message[64];
    memset(message, 0, sizeof(message));

    memcpy((void*) message + 0,       (void*)&header,  sizeof(header));
    memcpy((void*) message + 44,      (void*) checksum, sizeof(checksum));
    memcpy((void*) message + 44 + 16, (void*) footer,   sizeof(footer));

    ////////////////////////////////////////////////////////////////////////////
    // send message
    ////////////////////////////////////////////////////////////////////////////

    seabreeze_write_usb(spec_index, &error, STS_REQUEST_ENDPOINT, message, sizeof(message));
    check_error(spec_index, &error, "seabreeze_write_usb");

    ////////////////////////////////////////////////////////////////////////////
    // read acknowledgement
    ////////////////////////////////////////////////////////////////////////////

    if (requestAck)
    {
        seabreeze_read_usb(spec_index, &error, STS_RESPONSE_ENDPOINT, message, sizeof(message));
        check_error(spec_index, &error, "seabreeze_read_usb");
    }

    return error;
}

void enableLampSTS(int enabled, unsigned int periodMillisec)
{
    // see com/oceanoptics/omnidriver/spectrometer/sts/STS.java:
    //
    //    public void setStrobeEnable(boolean enable) throws IOException {
    //        if(null != this.lastStrobeEnable && this.lastStrobeEnable.booleanValue() == enable) {
    //            /* Nothing to do, so spare the bus the overhead */
    //            return;
    //        }
    //        this.lastStrobeEnable = new Boolean(enable);
    //        setLampEnable(enable);                // 1
    //        setShutterOpen(enable);               // 2
    //        setSingleStrobeEnable(enable);        // 3
    //        setContinuousStrobeEnable(enable);    // 4
    //    }

    const unsigned int STS_MESSAGE_CONT_STROBE_PERIOD  = 0x00310010;
    const unsigned int STS_MESSAGE_CONT_STROBE_ENABLED = 0x00310011;
    const unsigned int STS_MESSAGE_TRIGGER_MODE        = 0x00110110;
    const unsigned int STS_MESSAGE_LAMP_ENABLE         = 0x00110410;
    const unsigned int STS_MESSAGE_SET_SHUTTER         = 0x00110610;  // in OmniDriver, not in Data Sheet?!?
    const unsigned int STS_MESSAGE_SET_SINGLE_STROBE   = 0x00300012;

    // 0: setTriggerMode(...)
    unsigned char triggerMode = 0;
    sendOBPMessage(STS_MESSAGE_TRIGGER_MODE, sizeof(triggerMode), &triggerMode);

    // 1: setLampEnable(enable)
    unsigned char lampEnable = enabled == 0 ? 0 : 1;
    sendOBPMessage(STS_MESSAGE_LAMP_ENABLE, sizeof(lampEnable), &lampEnable);

    // 2: setShutterOpen(enable)
    unsigned char shutterOpen = 1; // why ever close?
    sendOBPMessage(STS_MESSAGE_SET_SHUTTER, sizeof(shutterOpen), &shutterOpen);

    // 3: setSingleStrobeEnable(enable)
    unsigned char singleStrobeEnable = enabled == 0 ? 0 : 1;
    sendOBPMessage(STS_MESSAGE_SET_SINGLE_STROBE, sizeof(singleStrobeEnable), &singleStrobeEnable);

    // 4: setContinuousStrobeEnable(enable)
    unsigned char continuousStrobeState = enabled == 0 ? 0 : 1;
    sendOBPMessage(STS_MESSAGE_CONT_STROBE_ENABLED, sizeof(continuousStrobeState), &continuousStrobeState);

    // 5: setContinuousStrobePeriod(...)
    unsigned long periodUsec = periodMillisec * 1000;
    unsigned char pulseDelayUsecLittleEndian[4];
    pulseDelayUsecLittleEndian[0] = (periodUsec >>  0) & 0xff;
    pulseDelayUsecLittleEndian[1] = (periodUsec >>  8) & 0xff;
    pulseDelayUsecLittleEndian[2] = (periodUsec >> 16) & 0xff;
    pulseDelayUsecLittleEndian[3] = (periodUsec >> 24) & 0xff;
    sendOBPMessage(STS_MESSAGE_CONT_STROBE_PERIOD, sizeof(pulseDelayUsecLittleEndian), pulseDelayUsecLittleEndian);
}

int readBool(const char *prompt)
{
    char c = 0;
    printf("%s", prompt);
    fflush(stdin);
    while (c != 'Y' && c != 'y' && c != 'N' && c != 'n')
        scanf("%c", &c);
    fflush(stdin);
    return c == 'Y' || c == 'y';
}

void takeThrowawaySpectra()
{
    const unsigned int throwawayCount = 1;
    double d;

    for (int i = 0; i < throwawayCount; i++)
    {
        seabreeze_get_formatted_spectrum(spec_index, &error, &d, 1);
        check_error(spec_index, &error, "seabreeze_get_formatted_spectrum");
    }
}

void setIntegrationTime()
{
    seabreeze_set_integration_time_microsec(spec_index, &error, 100 * 1000);  // 100ms
    check_error(spec_index, &error, "seabreeze_set_integration_time_microsec");
}

int main(int argc, char **argv)
{
    char ver[256];
    seabreeze_get_api_version_string(ver, sizeof(ver));
    printf("SeaBreeze API Version: %s\n", ver);

    printf("Testing STS/Spark lamp features.\n\n");

    seabreeze_open_spectrometer(spec_index, &error);
    if (check_error(spec_index, &error, "seabreeze_open_spectrometer"))
        return 1;

    char spec_type[16];
    seabreeze_get_model(spec_index, &error, spec_type, sizeof(spec_type));
    if (check_error(spec_index, &error, "seabreeze_get_model"))
        return 1;

    if (strncmp(spec_type, "STS", 3) && strncmp(spec_type, "SPARK", 5))
    {
        printf("WARNING: This program is only designed to test Ocean Optics STS or Spark spectrometers!\n");
        printf("         Proceed at your own risk...\n\n");
    }

    enableLampSTS(0, 0);

    int done = 0;
    while (!done)
    {
        unsigned long strobe_period_millisec = 0;

        // get settings for this test pass
        int enableLamp = readBool("Enable lamp? [yn] ");
        if (enableLamp)
        {
            printf("  Strobe interval in milliseconds: ");
            scanf("%lu", &strobe_period_millisec);
        }

        // apply settings
        seabreeze_set_verbose(1);

        setIntegrationTime();

        enableLampSTS(enableLamp, strobe_period_millisec);

        seabreeze_set_verbose(0);

        takeThrowawaySpectra();

        printf("All settings applied successfully\n\n");

        // prepare for next pass
        done = readBool("Quit? [yn]");
    }

    enableLampSTS(0, 0);

    seabreeze_close_spectrometer(spec_index, &error);
    if (check_error(spec_index, &error, "seabreeze_close_spectrometer"))
        return 1;

    seabreeze_shutdown();

    return 0;
}
