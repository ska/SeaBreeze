/**
* @file     demo-stability-scan.cpp
* @author   Mark Zieg <mark.zieg@oceanoptics.com>
* @date     2012-01-17
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
#include <math.h>
#include <time.h>

#include "api/SeaBreezeWrapper.h"
#include "util.h"

#define MAX_LABEL_SIZE 15
#define MS_TO_USEC 1000

#ifndef MAXFLOAT
#define MAXFLOAT	3.40282347e+38F
#endif

// if two successive scans vary in average height by more than this relative
// percentage (i.e. 5%), then consider it a flicker of instability
#define FLICKER_THRESHOLD 0.05

// global for convenience
int spec_index = 0;
int error = 0;

// generate quick min/max/avg statistics on a spectrum, for comparison purposes
void getStats(const double* A, unsigned len, double *min, double *max, double *avg) {
    *min = MAXFLOAT;
    *max = MAXFLOAT * -1;
    double sum = 0.f;
    for (unsigned i = 0; i < len; i++) {
        sum += A[i];
        *min = A[i] < *min ? A[i] : *min;
        *max = A[i] > *max ? A[i] : *max;
    }
    *avg = sum / len;
}

// determine whether two successive scans appear stable, or seem to "flicker"
int checkFlicker(const unsigned i, const char* label, const double* x, const double* y, unsigned len) {
    printf("Iteration %03u-%s: ", i, label);
    if (!i) {
        printf("initial scan\n");
        return 0;   // skip first call (no prev)
    }

    double minX, maxX, avgX, minY, maxY, avgY;
    getStats(x, len, &minX, &maxX, &avgX);   // technically we could cache this set
    getStats(y, len, &minY, &maxY, &avgY);

    // determine whether the scans flicker by comparing the average photon counts
    double variance = fabs(avgX - avgY) / avgX;
    if (variance > FLICKER_THRESHOLD) {
        printf("FLICKER (old min %le / avg %le / max %le)\n", minX, avgX, maxX);
        printf("                         (new min %le / avg %le / max %le)\n", minY, avgY, maxY);
        return 1;
    } else {
        printf("stable\n");
        return 0;
    }
}

int main(int argc, char **argv) {

    ////////////////////////////////////////////////////////////////////////////
    // initialization
    ////////////////////////////////////////////////////////////////////////////

    int device_count = 0;

    logger_header("Output from %s", argv[0]);

    logger("Finding connected spectrometers...");
    for(int i = 0; i < SEABREEZE_MAX_DEVICES; i++)
        if(!seabreeze_open_spectrometer(i, &error))
            device_count++;
        else
            break;
    logger("%d devices found.", device_count);
    if (!device_count)
        exit(1);

    logger("");
    printf("Defaulting to spectrometer spec_index: %d\n", spec_index);

    char type[MAX_LABEL_SIZE + 1];
    seabreeze_get_model(spec_index, &error, type, sizeof(type));
    if (check_error(spec_index, &error, "getSpectrometerType"))
        exit(1);
    logger("Spectrometer type: %s", type);

    int len = seabreeze_get_formatted_spectrum_length(spec_index, &error);
    if (check_error(spec_index, &error, "getFormattedSpectrumLength"))
        exit(1);
    logger("Formatted spectrum length = %d doubles", len);

    ////////////////////////////////////////////////////////////////////////////
    // Prompt user to enter test parameters
    ////////////////////////////////////////////////////////////////////////////

    printf("Enter integration time 'A' in milliseconds (e.g. 4): ");
    unsigned long integ1 = 0;
    scanf("%lu", &integ1);

    printf("Enter integration time 'B' in milliseconds (e.g. 100): ");
    unsigned long integ2 = 0;
    scanf("%lu", &integ2);

    printf("Add deliberate noise to force flickers (y/n): ");
    fflush(stdin);
    char c;
    do {
        c = getchar();
    } while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
    int addNoise = c == 'Y' || c == 'y';

    printf("Enter the number of iterations (e.g. 100): ");
    unsigned iterations = 0;
    scanf("%u", &iterations);

    printf("\n"
           "We will now take %u paired acquisitions (total of %u scans), toggling\n"
           "between integration time 'A' (%lums) and integration time 'B' (%lums).\n"
           "After each change in integration time, we will command a single throwaway\n"
           "stability scan (essentially taking two scans in a row for each integration\n"
           "time, or four scans per iteration).\n"
           "\n"
           "What we are looking for are abrupt changes in average graph height (Y value)\n"
           "between successive scans of either spectrum A or spectrum B.  While spectrums\n"
           "A and B are expected to differ due to their different integration times,\n"
           "successive scans using the same integration time should be fairly similar,\n"
           "especially when using a light source.  If we see one spectrum (probably B)\n"
           "flickering between high and low ranges, then something may be amiss in\n"
           "our commanded stability scans.\n\n",
           iterations, iterations * 2, integ1, integ2);

    ////////////////////////////////////////////////////////////////////////////
    // Execute test
    ////////////////////////////////////////////////////////////////////////////

    unsigned bytes = len * sizeof(double);
    double specA[len]; memset(specA, 0, bytes);
    double specB[len]; memset(specB, 0, bytes);
    double prevA[len]; memset(prevA, 0, bytes);
    double prevB[len]; memset(prevB, 0, bytes);
    int flickersA = 0;
    int flickersB = 0;
    srand(time(NULL));

    for (unsigned i = 0; i < iterations; i++) {

        // set integration time A
        seabreeze_set_integration_time_microsec(spec_index, &error, integ1 * MS_TO_USEC);
        check_error(i, &error, "setIntegrationTime(A)");

        // take stability scan A
        seabreeze_get_formatted_spectrum(spec_index, &error, specA, len);
        check_error(i, &error, "takeStabilityScan(A)");

        // take acquisition A
        seabreeze_get_formatted_spectrum(spec_index, &error, specA, len);
        check_error(i, &error, "getFormattedSpectrum(A)");

        // compare A to A'
        flickersA += checkFlicker(i, "A", prevA, specA, len);

        // set integration time B
        seabreeze_set_integration_time_microsec(spec_index, &error, integ2 * MS_TO_USEC);
        check_error(i, &error, "setIntegrationTime(A)");

        // take stability scan B
        seabreeze_get_formatted_spectrum(spec_index, &error, specB, len);
        check_error(i, &error, "takeStabilityScan(A)");

        // take acquisition B
        seabreeze_get_formatted_spectrum(spec_index, &error, specB, len);
        check_error(i, &error, "getFormattedSpectrum(B)");

        if (addNoise) {
            if (((1.0 * rand()) / RAND_MAX) < 0.25) {
                for (unsigned j = 0; j < len; j++) {
                    double r = 1.0 + (1.0 * rand()) / RAND_MAX;
                  //printf("[%03d]   %u: scaling by %+5.3f\n", i, j, r);
                    specB[j] *= r;
                }
            }
        }

        // compare B to B'
        flickersB += checkFlicker(i, "B", prevB, specB, len);

        // cleanup
        memcpy(prevA, specA, bytes);
        memcpy(prevB, specB, bytes);
        memset(specA, 0, bytes);
        memset(specB, 0, bytes);
        fflush(stdout);
    }

    printf ("Over %d iterations, encountered %d likely flickers (%d in A, %d in B).\n",
        iterations, flickersA + flickersB, flickersA, flickersB);
    return (flickersA || flickersB) ? 1 : 0;
}
