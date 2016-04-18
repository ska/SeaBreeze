/**************************************************************************//**
 * @file    demo-timing.c
 * @date    21-Nov-2012
 * @author  Ocean Optics, Inc.
 * @brief   Demonstrate how to time SeaBreeze calls.
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
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "api/SeaBreezeWrapper.h"
#include "util.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LABEL_SIZE     15

int main(int argc, char **argv)
{
    int error = 0;
    logger_header("Output from %s", argv[0]);

    ////////////////////////////////////////////////////////////////////////////
    // open spectrometer
    ////////////////////////////////////////////////////////////////////////////

    int specIndex = 0;
    if(seabreeze_open_spectrometer(specIndex, &error))
    {
        logger("no devices found.");
        exit(1);
    }

    ////////////////////////////////////////////////////////////////////////////
    // describe the unit we're testing
    ////////////////////////////////////////////////////////////////////////////

    char type[MAX_LABEL_SIZE + 1];
    seabreeze_get_model(specIndex, &error, type, sizeof(type));
    if (check_error(specIndex, &error, "seabreeze_get_model"))
        exit(1);

    int pixels = seabreeze_get_formatted_spectrum_length(specIndex, &error);
    if (check_error(specIndex, &error, "seabreeze_get_formatted_spectrum_length"))
        exit(1);

    logger("Testing index 0 (%s with %d pixels)", type, pixels);

    ////////////////////////////////////////////////////////////////////////////
    // get test parameters
    ////////////////////////////////////////////////////////////////////////////

    unsigned integ_time_millis = 0;
    printf("\nEnter integration time (millisec): ");
    scanf("%u", &integ_time_millis);

    unsigned scans_to_average = 0;
    printf("Enter scans to average: ");
    scanf("%u", &scans_to_average);

    unsigned iterations = 0;
    printf("Enter number of iterations: ");
    scanf("%u", &iterations);

    printf("\n");

    ////////////////////////////////////////////////////////////////////////////
    // configure test
    ////////////////////////////////////////////////////////////////////////////

    seabreeze_set_integration_time_microsec(specIndex, &error, integ_time_millis * 1000);
    check_error(specIndex, &error, "seabreeze_set_integration_time_microsec");

    double *spectrum = (double*) malloc(pixels * sizeof(double));
    double *average  = (double*) malloc(pixels * sizeof(double));

    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);

    ////////////////////////////////////////////////////////////////////////////
    // run test
    ////////////////////////////////////////////////////////////////////////////

    logger("Running test");
    for (unsigned i = 0; i < iterations; i++)
    {
        logger("  Iteration %u of %u", i + 1, iterations);
        memset(average, 0, pixels * sizeof(double));
        for (int j = 0; j < scans_to_average; j++) {
            logger("    Scan %u of %u", j + 1, scans_to_average);
            memset(spectrum, 0, pixels * sizeof(double));
            seabreeze_get_formatted_spectrum(specIndex, &error, spectrum, pixels);
            for (unsigned k = 0; k < pixels; k++)
                average[k] += spectrum[k];
        }
        for (unsigned k = 0; k < pixels; k++)
            average[k] /= scans_to_average;
    }
    logger("Test complete");

    ////////////////////////////////////////////////////////////////////////////
    // report elapsed time
    ////////////////////////////////////////////////////////////////////////////

    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);

    unsigned sec = tv_end.tv_sec - tv_start.tv_sec;
    long micros = tv_end.tv_usec - tv_start.tv_usec;
    if (micros < 0)
    {
        micros += 1000000L;
        sec++;
    }

    double elapsed_sec = sec + (micros / 1000000.0);
    double expected_sec = iterations * integ_time_millis * scans_to_average / 1000.0;

    logger("Test took %lf sec (%lf expected)", elapsed_sec, expected_sec);

    ////////////////////////////////////////////////////////////////////////////
    // cleanup
    ////////////////////////////////////////////////////////////////////////////

    free(average);
    free(spectrum);
    seabreeze_close_spectrometer(specIndex, &error);
}
