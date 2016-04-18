/**************************************************************************//**
 * @file    demo-unformatted-spectrum.c
 * @date    Nov 2011
 * @author  Ocean Optics, Inc.
 * @brief   Demonstrate how to process an unformatted spectrum.
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
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "api/SeaBreezeWrapper.h"
#include "util.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LABEL_SIZE 15
#define MAX_DEVICES    16    // max number of connected spectrometers
#define MAX_PIXELS     4096  // highest for any supported device
#define SYNCH_BYTE     0x069 // from HR4000 product spec
#define MILLIS_TO_USEC 1000

#ifndef M_PI
#define M_PI		   3.14159265358979323846
#endif

int error = 0;      // global for convenience error-handling function

int get_unformatted_spectrum_doubles(int index, int raw_length, double *buffer) {

    // get unformatted spectrum
    unsigned char *bSpectrum = (unsigned char*) malloc(raw_length);
    int bytes_read = seabreeze_get_unformatted_spectrum(index, &error, bSpectrum, raw_length);
    if (bytes_read < 0 || error) {
        logger("[%02d] Error calling seabreeze_get_unformatted_spectrum: %s",
            index, get_error_string(error));
        return -1;
    }

    // strip synchronization byte if present (not all devices have one)
    if (raw_length % 2) {
        if (bSpectrum[raw_length - 1] != SYNCH_BYTE) {
            logger("[%02d] Error: trailing byte %d (%02x) != synch byte (%02x)",
                index, raw_length, bSpectrum[raw_length - 1], SYNCH_BYTE);
            free((void*) bSpectrum);
            return -1;
        }
        raw_length--;  // ignore last byte hereafter
    }

    // convert unformatted bytes into array of doubles
    int pixels = raw_length / 2;
    for (int j = 0; j < pixels; j++) {
        unsigned char lsb = bSpectrum[j*2];
        unsigned char msb = bSpectrum[j*2 + 1];
        unsigned counts = (msb << 8) + lsb;
        buffer[j] = counts;
        logger("[%02d] Unformatted pixel %4d: 0x%02x 0x%02x = 0x%04x = %8u dec = %8.2lf", index, j, lsb, msb, counts, counts, buffer[j]);
    }
    free((void*) bSpectrum);
    return 0;
}

int main(int argc, char **argv) {

    int device_count = 0;

    logger_header("Output from %s", argv[0]);

    seabreeze_set_verbose(1);

    logger("Finding connected spectrometers...");
    for(int i = 0; i < SEABREEZE_MAX_DEVICES; i++)
        if(!seabreeze_open_spectrometer(i, &error))
            device_count++;
        else
            break;
    logger("%d devices found.", device_count);
    logger("");

    // test each connected device
    for (int i = 0; i < device_count; i++) {

        char label[80];

        ////////////////////////////////////////////////////////////////////////
        // read spectrometer configuration
        ////////////////////////////////////////////////////////////////////////

        char type[MAX_LABEL_SIZE + 1];
        seabreeze_get_model(i, &error, type, sizeof(type));
        if (check_error(i, &error, "seabreeze_get_model"))
            continue;

        logger("[%02d] Spectrometer type: %s", i, type);

        ////////////////////////////////////////////////////////////////////////
        // initialize spectrometer to common baseline
        ////////////////////////////////////////////////////////////////////////

        logger("[%02d] Initializing spectrometer", i);

        logger("[%02d]   setting integration time (100ms)", i);
        seabreeze_set_integration_time_microsec(i, &error, 100 * MILLIS_TO_USEC);
        check_error(i, &error, "seabreeze_set_integration_time_microsec");

        int default_trigger_mode = 0;
        logger("[%02d]   setting default trigger mode (%d)", i, default_trigger_mode);
        seabreeze_set_trigger_mode(i, &error, default_trigger_mode);
        if (check_error(i, &error, "seabreeze_set_trigger_mode"))
            continue;

        logger("[%02d]   generating throwaway acquisition following initialization", i);

        int formatted_length = seabreeze_get_formatted_spectrum_length(i, &error);
        if (check_error(i, &error, "seabreeze_get_formatted_spectrum_length"))
            continue;
        logger("[%02d]   formatted spectrum length = %d doubles", i, formatted_length);
        double *dFormatted= (double*) malloc(formatted_length * sizeof(double));

        int raw_length = seabreeze_get_unformatted_spectrum_length(i, &error);
        int pixels = raw_length / 2;
        if (check_error(i, &error, "seabreeze_get_unformatted_spectrum_length"))
            continue;
        logger("[%02d]   unformatted spectrum length = %d bytes (%d pixels)", i, raw_length, pixels);
        double* dUnformatted = (double*) malloc(raw_length/2 * sizeof(double));

        seabreeze_get_formatted_spectrum(i, &error, dFormatted, pixels);
        if (check_error(i, &error, "seabreeze_get_formatted_spectrum"))
            continue;

        logger("[%02d]   Ready to begin initialized spectrometer testing", i);
        logger("");

        ////////////////////////////////////////////////////////////////////////
        // get unformatted spectrum
        ////////////////////////////////////////////////////////////////////////

        if (!get_unformatted_spectrum_doubles(i, raw_length, dUnformatted)) {
            graph(type, "UNFORMATTED", dUnformatted, pixels);
        } else {
            logger("[%02] Error attempting to fetch unformatted spectrum as doubles", i);
            continue;
        }

        ////////////////////////////////////////////////////////////////////////
        // perform multi-scan averaging
        ////////////////////////////////////////////////////////////////////////

        // take a set of five raw acquisitions
        #define SCAN_COUNT 5
        double scans[SCAN_COUNT][MAX_PIXELS];
        memset(scans, 0, sizeof(scans));
        for(unsigned j = 0; j < SCAN_COUNT; j++) {
            if (get_unformatted_spectrum_doubles(i, raw_length, &(scans[j][0]))) {
                logger("[%02] Error attempting to fetch unformatted spectrum as doubles", i);
                break;
            }

            // MZ: I no longer recall why this was considered useful, other than
            // proving that multiple, DIFFERENT spectra are in fact being read, such
            // that the averaged result will demonstrably vary from the source data.
            if (0)
            {
                // weight each scan with a sinusoid, shifting each by a distinct phase
                float phi = M_PI + j * M_PI / SCAN_COUNT;
                for (unsigned k = 0; k < pixels; k++) {
                    float a = k * 2 * M_PI / pixels;
                    scans[j][k] *= 1 + cos( a + phi );
                }
            }

            snprintf(label, sizeof(label), "RAW #%d", j + 1);
            graph(type, label, scans[j], pixels);
        }

        ////////////////////////////////////////////////////////////////////////
        // sanity-check: by way of comparison, display the formatted equivalents
        ////////////////////////////////////////////////////////////////////////

        seabreeze_get_formatted_spectrum(i, &error, dFormatted, pixels);
        check_error(i, &error, "seabreeze_get_formatted_spectrum");
        graph(type, "FORMATTED", dFormatted, pixels);

        for (unsigned i = 0; i < pixels; i++)
            printf("pixel %04d: UNFORMATTED %8.2lf FORMATTED %8.2lf\n", i, dUnformatted[i], dFormatted[i]);

        free((void*) dUnformatted);
        free((void*) dFormatted);

        seabreeze_close_spectrometer(i, &error);
        check_error(i, &error, "seabreeze_close_spectrometer");
    }

    return 0;
}
