/*******************************************************
 * File:    spectral_correction.c
 * Date:    March 2010
 * Author:  Ocean Optics, Inc.
 *
 * This implements some simple spectral corrections for
 * use with SeaBreeze.
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

#include <stdlib.h>

#include "api/SeaBreezeWrapper.h"

#define MAX_DARK_PIXELS 32

int is_electric_dark_supported(int index, int *error_code) {
    int test;
    int flag;

    /* Try to read out one electric dark pixel index from the device.  If this
     * does not copy anything, zero will be returned and the spectrometer has
     * no electric dark pixels.  Otherwise, it does have them.
     */
    flag = seabreeze_get_electric_dark_pixel_indices(index, error_code, &test, 1);
    return flag;
}

/* Note: this is provided for illustration only.  This is inefficient because
 * it will read out the array of dark pixels on every call.  Ideally, the
 * program would read out the electric dark pixel indices on startup and
 * cache them for later use.  Also note that better stability over time may
 * be achieved by keeping the last several baseline estimates in a circular
 * buffer and averaging them to create an overall average.  This will make the
 * baseline estimate less sensitive to noise.
 */
int get_edark_corrected_spectrum(int index, int *error_code, double *buffer,
        int buffer_length) {
    int dark_pixels[MAX_DARK_PIXELS];
    int dark_pixel_count;
    int i;
    int pixel_count;
    double baseline = 0;  /* estimated from electric dark pixels */

    dark_pixel_count = seabreeze_get_electric_dark_pixel_indices(index,
        error_code, dark_pixels, MAX_DARK_PIXELS);

    if(0 == dark_pixel_count) {
        /* Device has no electric dark pixels, so bail out. */
        return 0;
    }

    pixel_count = seabreeze_get_formatted_spectrum(index, error_code, buffer,
        buffer_length);

    for(i = 0; i < dark_pixel_count; i++) {
        baseline += buffer[dark_pixels[i]];
    }
    baseline /= (double)dark_pixel_count;

    for(i = 0; i < pixel_count; i++) {
        buffer[i] -= baseline;
    }
    return pixel_count;
}

/* Note: this is provided for illustration only.  This is inefficient because it
 * tries to read out the EEPROM slots for the nonlinearity calibration on
 * every call.  Ideally, these should be read out once when the program starts
 * and cached for later use.
 *
 * This function assumes that the pixel array has an approximately zero baseline.
 * The spectrum must be pre-processed somehow: either with electric dark correction,
 * or the calling application must have an appropriate dark spectrum stored that
 * it can subtract from the sample before calling this function.  Using both
 * electric dark correction and subtracting a stored dark (where the dark spectrum
 * was also corrected for electric dark) is recommended.
 */
int do_nonlinearity_correction(int index, int *error_code,
        double *pixels, double pixel_count) {
    int i;
    int pixel;
    int slot_index;
    int copied;
    unsigned char slot_bytes[24] = { 0 };
    float nl_coeffs[8] = { 0 };
    double xpower;
    double y;

    for(i = 0, slot_index = 6; i < 8; i++, slot_index++) {
        copied = seabreeze_read_eeprom_slot(index, error_code, slot_index,
            slot_bytes, 24);
        if(0 == copied) {
            /* Failed to read the EEPROM, so bail out. */
            return 0;
        }
        slot_bytes[copied] = '\0';  /* Make sure the string is terminated */
        nl_coeffs[i] = atof((char *)slot_bytes);
    }

    if(nl_coeffs[0] == 0 || nl_coeffs[0] == nl_coeffs[1]) {
        /* This indicates that correct coefficients were probably not programmed
         * into the device.  Rather than divide by zero below, this will just
         * bail out.
         */
        return 0;
    }

    for(pixel = 0; pixel < pixel_count; pixel++) {
        /* Evaluate the 7th order polynomial for each pixel's intensity and
         * apply the nonlinearity correction.
         */
        xpower = pixels[pixel];
        y = nl_coeffs[0];
        for(i = 1; i < 8; i++) {
            y += xpower*nl_coeffs[i];
            xpower *= pixels[pixel];
        }
        pixels[pixel] /= y;
    }
    return pixel_count;
}
