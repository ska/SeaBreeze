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

#include "api/seabreezeapi/SeaBreezeAPI.h"

// global for convenience
const int spec_index = 0;
int error = 0;
const int MAX_EEPROM_QUERY_RESPONSE = 17;  // per USB2000+ Data Sheet, p18

void dump_eeproms(long deviceID, long featureID, unsigned char sendEndpoint, unsigned char receiveEndpoint)
{
    printf("EEPROMs:\n");
    unsigned char request[2];
    unsigned char response[32];
    for (int i = 0; i < 20; i++)
    {
        request[0] = 0x05;  // query information
        request[1] = i;     // EEPROM slot

        memset(response, 0, sizeof(response));

        sbapi_raw_usb_bus_access_write(deviceID, featureID, &error, request, sizeof(request), sendEndpoint);
        if (check_error(deviceID, &error, "seabreeze_write_usb"))
            break;

        int bytesRead = sbapi_raw_usb_bus_access_read(deviceID, featureID, &error, response, MAX_EEPROM_QUERY_RESPONSE, receiveEndpoint);
        if (check_error(deviceID, &error, "seabreeze_read_usb"))
            break;

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

void dump_fpga_registers(long deviceID, long featureID, unsigned char sendEndpoint, unsigned char receiveEndpoint)
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
        unsigned char address = addresses[i++];

        request[0] = 0x6b;
        request[1] = address;

        memset(response, 0, sizeof(response));

        sbapi_raw_usb_bus_access_write(deviceID, featureID, &error, request, sizeof(request), sendEndpoint);
        if (check_error(deviceID, &error, "seabreeze_write_usb"))
            break;

        sbapi_raw_usb_bus_access_read(deviceID, featureID, &error, response, sizeof(response), receiveEndpoint);
        if (check_error(deviceID, &error, "seabreeze_read_usb"))
            break;

        unsigned short value = (unsigned short) (response[1] | (response[2] << 8));

        printf("  Register 0x%02x: hex 0x%04x dec %5hu (%s)\n", address, value, value, labels[i]);

    }
}


int main(int argc, char **argv)
{
	int result = 0;
	int number_of_devices;
	int error = 0;
    char nameBuffer[80];
    long *device_ids;
    long featureID;
    int i, flag;
        
	sbapi_initialize();

    printf("Probing for devices...\n");
    sbapi_probe_devices();
	
	printf("Getting device count...\n");
    number_of_devices = sbapi_get_number_of_device_ids();
    printf("Device count is %d\n", number_of_devices);
    
    if(number_of_devices>0)
    {
		printf("Getting device IDs...\n");
		device_ids = (long *)calloc(number_of_devices, sizeof(long));
		number_of_devices = sbapi_get_device_ids(device_ids, number_of_devices);
		printf("Got %d device ID%s.\n", number_of_devices, number_of_devices == 1 ? "" : "s");
		
		for(i = 0; i < number_of_devices; i++) 
		{
			printf("Device id=0x%02lX:\n", device_ids[i]);
			printf("\tGetting device type...\n");
			flag = sbapi_get_device_type(device_ids[i], &error, nameBuffer, 79);
			printf("\t\tResult is (%d) [%s]\n", flag, sbapi_get_error_string(error));
			if(flag > 0) 
			{
				unsigned char usb_ep_primary_out, usb_ep_primary_in, usb_ep_secondary_out, usb_ep_secondary_in, usb_ep_secondary_in2;
				
				printf("\tDevice type: [%s]\n", nameBuffer);
				
				// show the available USB endpoints for this device
				usb_ep_primary_out = sbapi_get_device_usb_endpoint_primary_out(device_ids[i], &error);
				if(usb_ep_primary_out!=0)
					printf("\t\t USB Primary Write Endpoint = 0x%02X\n", usb_ep_primary_out);
					
				usb_ep_primary_in = sbapi_get_device_usb_endpoint_primary_in(device_ids[i], &error);
				if(usb_ep_primary_in!=0)
					printf("\t\t USB Primary Read Endpoint = 0x%02X\n", usb_ep_primary_in);
					
				usb_ep_secondary_out = sbapi_get_device_usb_endpoint_secondary_out(device_ids[i], &error);
				if(usb_ep_secondary_out!=0)
					printf("\t\t USB Secopndary Write Endpoint = 0x%02X\n", usb_ep_secondary_out);
					
				usb_ep_secondary_in = sbapi_get_device_usb_endpoint_secondary_in(device_ids[i], &error);
				if(usb_ep_secondary_in!=0)
					printf("\t\t USB Secondary Read Endpoint = 0x%02X\n", usb_ep_secondary_in);
					
				usb_ep_secondary_in2 = sbapi_get_device_usb_endpoint_secondary_in2(device_ids[i], &error);
				if(usb_ep_secondary_in2!=0)
					printf("\t\t USB Secondary Alternate Read Endpoint = 0x%02X\n", usb_ep_secondary_in2);
				
				 /* Open the device */
        		printf("\tAttempting to open:\n");
        		flag = sbapi_open_device(device_ids[i], &error);
        		printf("\t\tResult is (%d) [%s]\n", flag, sbapi_get_error_string(error));
        
        		// if there was a problem, start the next iteration and skip the tests
        		if(flag != 0) 
        		{
            		continue;
        		}

				// get the raw usb bus access feature (there should be only one)
				if (sbapi_get_number_of_raw_usb_bus_access_features(device_ids[i], &error) == 1)
				{
				    if (! check_error(device_ids[i], &error, "sbapi_get_number_of_raw_usb_bus_access_features"))
					{
						if (sbapi_get_raw_usb_bus_access_features(device_ids[i], &error, &featureID, 1) == 1)
						{
							if (! check_error(device_ids[i], &error, "sbapi_get_raw_usb_bus_access_features"))
							{
								// run some USB tests
								dump_eeproms(device_ids[i], featureID, usb_ep_primary_out, usb_ep_primary_in);
				
								dump_fpga_registers(device_ids[i], featureID, usb_ep_primary_out, usb_ep_primary_in);
							}
						}
					}
				}

				
				/* Close the device */
				printf("\tAttempting to close:\n");
				sbapi_close_device(device_ids[i], &error);
				printf("\t\tResult is (%d) [%s]\n", flag, sbapi_get_error_string(error));
        	}
        	
        	 
		}
		free(device_ids);
    }
	return result;
}
