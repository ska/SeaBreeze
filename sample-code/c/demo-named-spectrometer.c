/*******************************************************
 * File:    demo-named-spectrometer.c
 * Date:    Nov 2011
 * Author:  Ocean Optics, Inc.
 *
 * Demonstrate how to process a named spectrometer (via
 * serial number or model), regardless of index position.
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
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include "api/SeaBreezeWrapper.h"
#include "util.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LABEL_SIZE  15  // largest length of any spectrometer type or serial number
#define MAX_DEVICES     16  // max number of connected spectrometers

void usage() {
    puts("Usage: demo-named-spectrometer [-t type_name] [-s serial_number]\n"
         "Examples:\n"
         "  $ demo-named-spectrometer -t STS\n"
         "  $ demo-named-spectrometer -s STSD00499\n"
         "  $ demo-named-spectrometer -t STS -s STSD00499");
    exit(1);
}

int main(int argc, char **argv) {

    char *desired_type = NULL;              // what spectrometer model we want
    char *desired_serial_number = NULL;     // what spectrometer SN we want
    int   matching_devices[MAX_DEVICES];    // indices of matching devices
    int   connected_device_count = 0;       // how many spectrometers found
    int   matching_device_count = 0;        // how many matching devices found
    int   error = 0;                        // SeaBreeze error result code

    ////////////////////////////////////////////////////////////////////////////
    // process cmd-line args
    ////////////////////////////////////////////////////////////////////////////

    opterr = 0;
    int c = 0;
    while ((c = getopt(argc, argv, "s:t:")) != -1) {
        switch (c) {
            case 's': desired_serial_number = optarg; break;
            case 't': desired_type          = optarg; break;
            default : usage();
        }
    }
    if (opterr || !(desired_type || desired_serial_number))
        usage();    // bad argument, or no arguments at all

    ////////////////////////////////////////////////////////////////////////////
    // count connected devices
    ////////////////////////////////////////////////////////////////////////////

    logger_header("Output from %s", argv[0]);

    logger("Finding connected spectrometers...");
    int i = 0;
    for(i = 0; i < SEABREEZE_MAX_DEVICES; i++)
        if(0 == seabreeze_open_spectrometer(i, &error))
            connected_device_count++;
        else
            break;
    logger("%d devices found.", connected_device_count);
    logger("");

    ////////////////////////////////////////////////////////////////////////////
    // look for desired spectrometer
    ////////////////////////////////////////////////////////////////////////////

    // iterate over connected devices, attempting to match on model & serial
    logger("Looking for matching spectrometer(s)...");
    char label[MAX_LABEL_SIZE + 1];
    char type[MAX_LABEL_SIZE + 1];
    for(i = 0; i < connected_device_count; i++) {
        int potential_match = 1;

        // match on spectrometer type
        seabreeze_get_model(i, &error, type, sizeof(type));
        if (0 == error) {
            if (desired_type) {
                if (!strcmp(type, desired_type)) {
                    logger("  [%02d] matches desired spectrometer type '%s'", i, type);
                } else {
                    logger("  [%02d] does not match desired spectrometer type '%s' (%s)", i, desired_type, type);
                    potential_match = 0;
                }
            } else {
                logger("  [%02d] has spectrometer type '%s' (no match criteria provided)", i, type);
            }
        } else {
            logger("  [%02d]   Error getting type: %s", i, get_error_string(error));
            potential_match = 0;
        }

        // match on serial_number
        seabreeze_get_serial_number(i, &error, label, sizeof(label));
        if (0 == error) {
            if (desired_serial_number) {
                if (!strncmp(label, desired_serial_number, sizeof(label))) {
                    logger("  [%02d] matches desired serial number '%s'", i, label);
                } else {
                    logger("  [%02d] does not match desired serial number '%s' (%s)", i, desired_serial_number, label);
                    potential_match = 0;
                }
            } else {
                logger("  [%02d] has serial number '%s' (no match criteria provided)", i, label);
            }
        } else {
            logger("  [%02d]   Error getting serial number: %s", i, get_error_string(error));
            potential_match = 0;
        }

        // aggregate results
        if (potential_match) {
            logger("  [%02d] matched all parameters.", i);
            matching_devices[matching_device_count++] = i;
        } else {
            logger("  [%02d] failed to match your parameters (closing).", i);
            seabreeze_close_spectrometer(i, &error);
            if (0 != error) {
                logger("  [%02d]   Error closing spectrometer: %s", i, get_error_string(error));
            }
        }
        logger("");
    }

    // give up if no matches
    if (!matching_device_count) {
        logger("Sorry, no matching devices found.");
        return 1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Process matching devices
    ////////////////////////////////////////////////////////////////////////////

    // display matching index(es)
    logger("Found %d matching devices, at the following indices:", matching_device_count);
    for (i = 0; i < matching_device_count; i++)
        logger("  %d", matching_devices[i]);
    logger("");

    // take one acquisition from each matching device
    for (i = 0; i < matching_device_count; i++) {
        int index = matching_devices[i];
        int spec_length = 0;

        logger("Getting formatted spectrum from matching spectrometer #%d (index %02d).", i, index);
        spec_length = seabreeze_get_formatted_spectrum_length(index, &error);
        if (0 != error) {
            logger("  [%02d] Error getting spectrum length: %s", index, get_error_string(error));
        } else {
            double *spectrum = (double *) calloc((size_t) spec_length, sizeof(double));
            int pixels_read = seabreeze_get_formatted_spectrum(index, &error, spectrum, spec_length);
            logger("  [%02d] Read %d pixels (error code '%s').", index, pixels_read, get_error_string(error));
            free(spectrum);
        }
    }

    // cleanup
    for (i = 0; i < matching_device_count; i++) {
        int index = matching_devices[i];
        seabreeze_close_spectrometer(index, &error);
        if (0 != error) {
            logger("  [%02d] Error closing spectrometer: %s", index, get_error_string(error));
        }
    }

    return 0;
}
