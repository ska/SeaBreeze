/*******************************************************
 * File:    demo-sts-eeprom.c
 * Date:    2-Dec-2014
 * Author:  Ocean Optics, Inc.
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
#include <string.h>
#include <unistd.h>  
#include "api/seabreezeapi/SeaBreezeAPI.h"

int main() 
{
    sbapi_initialize();

    printf("Probing for devices...\n"); 
    sbapi_probe_devices();
    unsigned number_of_devices = sbapi_get_number_of_device_ids();

    printf("Found %u devices.\n", number_of_devices);
    if(!number_of_devices) 
        return 1;

    // get device IDs
    long* device_ids = (long *) malloc(number_of_devices * sizeof(long));
    sbapi_get_device_ids(device_ids, number_of_devices);

    // test each device
    for (unsigned i = 0; i < number_of_devices; i++) 
    {
        long id = device_ids[i];
        int error = 0;

        // get model 
        char model[80];
        memset(model, 0, sizeof(model));
        sbapi_get_device_type(device_ids[i], &error, model, sizeof(model) - 1);
        printf("\nDevice %u (0x%04lx) is a/n %s\n", i, id, model);
        
        // open device
        if (sbapi_open_device(device_ids[i], &error))
        {
            printf("Error opening device %u (0x%04lx)\n", i, id);
            continue;
        }

        // get serial number(s) associated with device
        int sn_count = sbapi_get_number_of_serial_number_features(id, &error);
        long* feature_ids = (long *) malloc(sn_count * sizeof(long));
        sbapi_get_serial_number_features(id, &error, feature_ids, sn_count);
        printf("Device %u (0x%04lx) has %u serial numbers:\n", i, id, sn_count);
        for (unsigned j = 0; j < sn_count; j++) 
        { 
            char sn[80];
            memset(sn, 0, sizeof(sn));
            sbapi_get_serial_number(id, feature_ids[j], &error, sn, sizeof(sn) - 1);
            printf("  %s\n", sn);
        }
        free(feature_ids);

        // get spectrometer(s) associated with device
        int spec_count = sbapi_get_number_of_spectrometer_features(id, &error);
        if(!spec_count)
        {
            printf("Error: can't find any spectrometers associated with device %u (%04lx)\n", i, id);
            continue;
        }

        long* spec_ids = (long *) malloc(spec_count * sizeof(long));
        sbapi_get_spectrometer_features(id, &error, spec_ids, spec_count);

        printf("Device %u (0x%04lx) has %u associated spectrometers:\n", i, id, spec_count);
        for (unsigned j = 0; j < spec_count; j++) 
        {
            // get wavelengths
            long spec_id = spec_ids[j];
            unsigned pixels = sbapi_spectrometer_get_formatted_spectrum_length(id, spec_id, &error);
            double* wavelengths = (double *) malloc(pixels * sizeof(double));
            sbapi_spectrometer_get_wavelengths(id, spec_id, &error, wavelengths, pixels);
            printf("    Spectrometer %u (0x%04lx) has %u pixels from %.2lf to %.2lf\n", j, spec_id, pixels, wavelengths[0], wavelengths[pixels - 1]);
            free(wavelengths);

            // get NLC
            unsigned number_of_nlc_features = sbapi_get_number_of_nonlinearity_coeffs_features(id, &error);
            if (number_of_nlc_features)
            {
                feature_ids = (long *) malloc(number_of_nlc_features * sizeof(long));
                sbapi_get_nonlinearity_coeffs_features(id, &error, feature_ids, number_of_nlc_features);
                printf("    Spectrometer %u (0x%04lx) has %u NLC features:\n", j, spec_id, number_of_nlc_features);
                for (unsigned k = 0; k < number_of_nlc_features; k++) 
                {
                    double coeffs[16]; // only 7 expected
                    memset(coeffs, 0, sizeof(coeffs));
                    unsigned coeff_count = sbapi_nonlinearity_coeffs_get(id, feature_ids[k], &error, coeffs, sizeof(coeffs));
                    printf("        NLC Feature %u (0x%04lx) has %u coefficients:\n", k, feature_ids[k], coeff_count);
                    for (unsigned kk = 0; kk < coeff_count; kk++)
                        printf("            Coeff %u: %.2e\n", kk, coeffs[kk]);
                }
                free(feature_ids);
            }
        }
        free(spec_ids);

        // close device
        sbapi_close_device(id, &error);
    }

    free(device_ids);
    sbapi_shutdown();
    return 0;
}
