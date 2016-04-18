/**
* @file     demo-streaming-xml.c
* @author   Mark Zieg <mark.zieg@oceanoptics.com>
* @date     2012-01-23
*
* Description: Demonstrate how to generate a series of streaming acquisitions
*              outputting spectra as XML to stdout (requested by a customer)
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

#define MAX_CONSECUTIVE_ERRORS 10
#define MILLISEC_TO_USEC 1000

// global for convenience
const int spec_index = 0;
int error = 0;

void openRec()  { puts("<spectrum>" ); }
void closeRec() { puts("</spectrum>"); }

void emitError(int errorCode, const char* func) {
    printf("<error function=\"%s\">%s</error>\n", func, get_error_string(errorCode));
}

void emitFatal(int errorCode, const char* func) {
    openRec();
    emitError(errorCode, func);
    closeRec();
    seabreeze_close_spectrometer(spec_index, &error);
    exit(1);
}

int main(int argc, char **argv) {

    ////////////////////////////////////////////////////////////////////////////
    // initialization
    ////////////////////////////////////////////////////////////////////////////

    printf("<?xml version=\"1.0\"?>\n");

    unsigned long integMS = 100;
    if (argc > 1)
        integMS = atoi(argv[1]);

    seabreeze_open_spectrometer(spec_index, &error);
    if (error)
        emitFatal(error, "seabreeze_open_spectrometer(0)");

    int len = seabreeze_get_formatted_spectrum_length(spec_index, &error);
    if (error)
        emitFatal(error, "seabreeze_get_formatted_spectrum_length");

    seabreeze_set_integration_time_microsec(spec_index, &error, integMS * MILLISEC_TO_USEC);
    if (error)
        emitFatal(error, "seabreeze_set_integration_time_microsec");

    unsigned bytes = len * sizeof(double);
    double spec[len];
    double wavelengths[len];
    memset(spec, 0, bytes);
    memset(wavelengths, 0, bytes);

    seabreeze_get_wavelengths(spec_index, &error, wavelengths, len);
    if (error)
        emitFatal(error, "seabreeze_get_wavelengths");

    ////////////////////////////////////////////////////////////////////////////
    // Execute test
    ////////////////////////////////////////////////////////////////////////////

    // Simply run until somebody unplugs the spectrometer or similar
    unsigned consecutive_errors = 0;
    while(consecutive_errors < MAX_CONSECUTIVE_ERRORS) {
        openRec();
        seabreeze_get_formatted_spectrum(spec_index, &error, spec, len);
        if (error) {
            emitError(error, "seabreeze_get_formatted_spectrum");
            consecutive_errors++;
        } else {
            consecutive_errors = 0;
            for (unsigned i = 0; i < len; i++) {
                printf("  <pixel index=\"%d\" wavelength=\"%le\" value=\"%le\" />\n",
                    i, wavelengths[i], spec[i]);
            }
        }
        closeRec();
        fflush(stdout);
    }
    emitFatal(error, "Shutting down after MAX_CONSECUTIVE_ERRORS");
}
