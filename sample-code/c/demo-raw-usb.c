/**
* @file     demo-raw-usb.c
* @author   Mark Zieg <mark.zieg@oceanoptics.com>
* @date     2012-10-16
*
* Description: Demonstrate how to exchange raw USB commands with a spectrometer.
*
* Note: This was mainly tested with a USB2000+.  Other spectrometer protocols may
*       vary; see your Data Sheet for details.
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
int error = 0;
const int MAX_EEPROM_QUERY_RESPONSE = 17;  // per USB2000+ Data Sheet, p18

// Unfortunately, for the time being you need to manually look these up in
// src/vendors/OceanOptics/features/raw_bus_access/RawUSBBusAccessFeature.cpp,
// as they typically differ by spectrometer.  These are the most common values,
// however.
const int SEND_ENDPOINT = 0x01;
const int READ_ENDPOINT = 0x81;

void dump_eeproms()
{
    printf("EEPROMs:\n");
    unsigned char request[2];
    unsigned char response[32];
    for (int i = 0; i < 20; i++)
    {
        request[0] = 0x05;  // query information
        request[1] = i;     // EEPROM slot

        memset(response, 0, sizeof(response));

        seabreeze_write_usb(spec_index, &error, SEND_ENDPOINT, request, sizeof(request));
        if (check_error(spec_index, &error, "seabreeze_write_usb"))
            continue;

        int bytesRead = seabreeze_read_usb(spec_index, &error, READ_ENDPOINT, response, MAX_EEPROM_QUERY_RESPONSE);
        if (check_error(spec_index, &error, "seabreeze_read_usb"))
            continue;

        printf("  EEPROM slot %02d:", i);
        for (int j = 0; j < bytesRead; j++)
            if (31 < response[j] && response[j] < 127)
                printf("%c", response[j]);
            else
                printf(".");
        printf("  ");
        for (int j = 0; j < bytesRead; j++)
            printf(" %02x", response[j]);
        printf("\n");
    }
}

void dump_fpga_registers()
{
    unsigned char addresses[] = {
        0x00, 0x04, 0x08, 0x0c,
        0x10, 0x14, 0x18, 0x20,
        0x28, 0x2c, 0x30, 0x38,
        0x3c, 0x40, 0x48, 0x50,
        0x54, 0x58, 0x5c, 0x60,
        0x64, 0x68, 0xFF }; // make sure last address is 0xFF
    const char* labels[] = {
        "Master Clock Counter Divisor", "FPGA Firmware Version", "Continuous Strobe Timer Internal Divisor", "Continuous Strobe Base Clock",
        "Integration Period Base Clock Divisor", "Set Base_Clk", "Integration Clock Timer Divisor", "Reserved",
        "Hardware Trigger Delay Number of Master Clock Cycles", "Trigger Mode", "Reserved", "Single Strobe High Clock",
        "Single Strobe Low Clock", "Lamp Enable", "GPIO Mux Register", "GPIO Output Enable",
        "GPIO Data Register", "Reserved", "Offset Value", "Offset Control",
        "FPGA Programmed", "Maximum Saturation Level", "LAST ADDRESS"
    };

    printf("FPGA Registers:\n");

    unsigned char request[2];
    unsigned char response[3];

    unsigned i = 0;
    while (addresses[i] != 0xff)
    {
        unsigned char address = addresses[i];

        request[0] = 0x6b;
        request[1] = address;

        memset(response, 0, sizeof(response));

        seabreeze_write_usb(spec_index, &error, SEND_ENDPOINT, request, sizeof(request));
        if (check_error(spec_index, &error, "seabreeze_write_usb"))
            continue;

        seabreeze_read_usb(spec_index, &error, READ_ENDPOINT, response, sizeof(response));
        if (check_error(spec_index, &error, "seabreeze_read_usb"))
            continue;

        unsigned short value = (unsigned short) (response[1] | (response[2] << 8));

        printf("  Register 0x%02x: hex 0x%04x dec %5hu (%s)\n", address, value, value, labels[i]);

        i++;
    }
}

int main(int argc, char **argv)
{
    // seabreeze_set_verbose(1);
    seabreeze_open_spectrometer(spec_index, &error);
    if (check_error(spec_index, &error, "seabreeze_open_spectrometer(0)"))
        return 1;

    dump_eeproms();

    dump_fpga_registers();

    seabreeze_close_spectrometer(spec_index, &error);
    if (check_error(spec_index, &error, "seabreeze_close_spectrometer(0)"))
        return 1;

    return 0;
}
