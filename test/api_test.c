/*******************************************************
 * File:    api_test.c
 * Date:    February 2012
 * Author:  Ocean Optics, Inc.
 *
 * This is a test program to exercise some of the
 * SeaBreeze functionality.  If using Windows, please
 * #define _WINDOWS (directly or as a project setting)
 * so that this will compile.
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

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api/seabreezeapi/SeaBreezeAPI.h"

#ifndef _WINDOWS
#include <unistd.h>
#else
#include <Windows.h>
#endif

/* Prototypes */
void test_serial_number_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_spectrometer_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_shutter_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_light_source_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_lamp_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_eeprom_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_irradcal_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_tec_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_nonlinearity_coeffs_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_temperature_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_spectrum_processing_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_revision_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_optical_bench_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_stray_light_coeffs_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_continuous_strobe_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_data_buffer_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_acquisition_delay_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);
void test_pixel_binning_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount);

/* Create a type called "testfunc_t" that is just a pointer to any function that
 * has this signature:  void func(long)
 * This will be used to invoke all of the test functions by reference.
 */
typedef void (*testfunc_t)(long, int*, int*);

/* Create a list of functions to run on each device that is found and opened */
static testfunc_t __test_functions[] = {
    test_serial_number_feature,
    test_revision_feature,
    test_optical_bench_feature,
    test_spectrometer_feature,
    test_shutter_feature,
    test_light_source_feature,
    test_lamp_feature,
    test_eeprom_feature,
    test_irradcal_feature,
    test_tec_feature,
    test_nonlinearity_coeffs_feature,
    test_temperature_feature,
    test_spectrum_processing_feature,
    test_stray_light_coeffs_feature,
    test_continuous_strobe_feature,
    test_data_buffer_feature,
    test_acquisition_delay_feature,
    test_pixel_binning_feature
};

/* Utilities to count errors and unsupported features */
 void tallyErrors(int error, int *testFailureCount)
 {
     if (error != 0) // 0 = no error
     {
         (*testFailureCount)++;
         printf("====>Log Failure, Count=%d\n", *testFailureCount);
     }
 }
 
  void tallyUnsupportedFeatures(int *unsupportedFeatureCount)
 {
    (*unsupportedFeatureCount)++;
    printf("------->Unsupported Feature, Count=%d\n", *unsupportedFeatureCount);
 }
 
/* Get a variable that tracks how many test functions have been specified */
static const int __test_function_count = sizeof(__test_functions)/sizeof(testfunc_t);

int main() {
    int number_of_devices;
    long *device_ids;
    int i;
    int test_index;
    int flag;
    int unsupportedFeatureCount;
    int testFailureCount;
    int error = 0;
    char nameBuffer[80];

    
    /* Give the driver a chance to initialize itself */
    sbapi_initialize();

    printf("Probing for devices...\n"); fflush(stdout);
    sbapi_probe_devices();

//#define RS232_TEST
#ifdef RS232_TEST
    printf("Adding an STS at 9600 baud...\n");
    /* Uncomment for Linux */
    //sbapi_add_RS232_device_location("STS", "/dev/ttyS0", 9600);
    //sbapi_add_RS232_device_location("STS", "/dev/ttyUSB0", 9600);

    /* Uncomment for Windows */
    //sbapi_add_RS232_device_location("STS", "COM1", 9600);

    /* Uncomment for e.g. USB-RS232 adapter under OSX */
    //sbapi_add_RS232_device_location("STS", "/dev/tty.KeySerial1", 9600);
    //sbapi_add_RS232_device_location("STS", "/dev/tty.usbserial", 9600);
#endif
    
    /* This shows how to add network devices (note that most use TCP/IP) */
    //sbapi_add_TCPIPv4_device_location("Jaz", "192.168.1.150", 7654);
    //sbapi_add_TCPIPv4_device_location("Blaze", "192.168.1.151", 57357);

    printf("Getting device count...\n"); fflush(stdout);
    number_of_devices = sbapi_get_number_of_device_ids();

    printf("Device count is %d\n", number_of_devices);
    if(0 == number_of_devices) {
        return 0;
    }

    printf("Getting device IDs...\n");
    device_ids = (long *)calloc(number_of_devices, sizeof(long));
    number_of_devices = sbapi_get_device_ids(device_ids, number_of_devices);
    printf("Got %d device ID%s.\n", number_of_devices, number_of_devices == 1 ? "" : "s");

    for(i = 0; i < number_of_devices; i++) {
        printf("%d: Device 0x%02lX:\n", i, device_ids[i]);
        printf("\tGetting device type...\n");
        flag = sbapi_get_device_type(device_ids[i], &error, nameBuffer, 79);
        printf("\t\tResult is (%d) [%s]\n", flag, sbapi_get_error_string(error));
        if(flag > 0) {
            printf("\tDevice type: [%s]\n", nameBuffer);
        }

        /* Open the device */
        printf("\tAttempting to open:\n");
        flag = sbapi_open_device(device_ids[i], &error);
        printf("\t\tResult is (%d) [%s]\n", flag, sbapi_get_error_string(error));
        
        // jump to the next iteration if there was a problem
        if(flag != 0) {
            continue;
        }

        // log deviations
        unsupportedFeatureCount=0;
        testFailureCount=0;
        
        /* Test the device */
        for(test_index = 0; test_index < __test_function_count; test_index++) {
            /* Invoke each of the test functions against this device */
            __test_functions[test_index](device_ids[i], &unsupportedFeatureCount, &testFailureCount);
        }

        /* Close the device */
        printf("\tAttempting to close:\n");
        sbapi_close_device(device_ids[i], &error);
        printf("\t\tResult is (%d) [%s]\n", flag, sbapi_get_error_string(error));
        printf("%d: Device 0x%02lX: \n\tNumber of unsupported features = %d\n\tNumber of test failures = %d\n", i, device_ids[i], unsupportedFeatureCount, testFailureCount);
    }
    free(device_ids);

    printf("Finished testing.\n");

    /* Clean up memory allocated by the driver */
    sbapi_shutdown();

    return 0;
}

void test_serial_number_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_serial_numbers;
    long *serial_number_ids = 0;
    int i;
    unsigned char length;
    char *buffer;

    printf("\n\tTesting serial number features:\n");

    printf("\t\tGetting number of serial numbers:\n");
    number_of_serial_numbers = sbapi_get_number_of_serial_number_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_serial_numbers, sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_serial_numbers) {
        printf("\tNo serial number capabilities found.\n");
        tallyUnsupportedFeatures(unsupportedFeatureCount);
        return;
    }

    serial_number_ids = (long *)calloc(number_of_serial_numbers, sizeof(long));
    printf("\t\tGetting serial number feature IDs...\n");    
    number_of_serial_numbers = sbapi_get_serial_number_features(deviceID, &error,
            serial_number_ids, number_of_serial_numbers);
    printf("\t\t\tResult is %d [%s]\n", number_of_serial_numbers,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for(i = 0; i < number_of_serial_numbers; i++) {
        printf("\t\t%d: Testing device 0x%02lX, serial number 0x%02lX\n",
                i, deviceID, serial_number_ids[i]);

        printf("\t\t\tAttempting to get the maximum serial number length...\n");
        length=sbapi_get_serial_number_maximum_length(deviceID, serial_number_ids[i], &error);
        printf("\t\t\tResult is %d [%s]\n", length, sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
            
        buffer = calloc(length+1, sizeof(char));
        
        printf("\t\t\tAttempting to get serial number...\n");
        //memset(buffer, (int)0, sizeof(buffer));
        sbapi_get_serial_number(deviceID, serial_number_ids[i], &error, buffer, length);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        if(0 == error) {
            printf("\t\t\t\tSerial number: [%s]\n", buffer);
        }
        
        free(buffer);
        printf("\t\t%d: Finished testing device 0x%02lX, serial number 0x%02lX\n",
                i, deviceID, serial_number_ids[i]);
    }

    free(serial_number_ids);

    printf("\tFinished testing serial number capabilities.\n");
}

void test_revision_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_revision_features;
    long *revision_feature_ids = 0;
    int i;

    printf("\n\tTesting revision features:\n");

    printf("\t\tGetting number of revision features:\n");
    number_of_revision_features = sbapi_get_number_of_revision_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_revision_features, sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_revision_features) {
        printf("\tNo revision capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    revision_feature_ids =
        (long *)calloc(number_of_revision_features, sizeof(long));
    printf("\t\tGetting revisino feature IDs...\n");
    number_of_revision_features = sbapi_get_revision_features(
            deviceID, &error, revision_feature_ids,
            number_of_revision_features);
    printf("\t\t\tResult is %d [%s]\n", number_of_revision_features,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for(i = 0; i < number_of_revision_features; i++) {
        printf("\t\t%d: Testing device 0x%02lX, revision 0x%02lX\n", i, deviceID, revision_feature_ids[i]);
        printf("\t\t\tAttempting to get revisions...\n");
        printf("\t\t\t\tHardware Revision: 0x%x\n", sbapi_revision_hardware_get(deviceID, revision_feature_ids[i], &error));
        printf("\t\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        printf("\t\t\t\tFirmware Revision: 0x%x\n", sbapi_revision_firmware_get(deviceID, revision_feature_ids[i], &error));
        printf("\t\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));    
        tallyErrors(error, testFailureCount);    
        
        printf("\t\t%d: Finished testing device 0x%02lX, revision 0x%02lX\n",
                i, deviceID, revision_feature_ids[i]);
    }
    free(revision_feature_ids);

    printf("\tFinished testing revision capabilities.\n");
}

void test_optical_bench_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_optical_benches;
    long *optical_bench_ids = 0;
    int i;
    unsigned int slit_or_fiber;
    char buffer[80];

    printf("\n\tTesting optical bench features:\n");

    printf("\t\tGetting number of optical benches:\n");
    number_of_optical_benches = sbapi_get_number_of_optical_bench_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_optical_benches, sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_optical_benches) {
        printf("\tNo optical bench capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    optical_bench_ids = (long *)calloc(number_of_optical_benches, sizeof(long));
    printf("\t\tGetting optical bench feature IDs...\n");
    number_of_optical_benches = sbapi_get_optical_bench_features(deviceID, &error, optical_bench_ids, number_of_optical_benches);
    printf("\t\t\tResult is %d [%s]\n", number_of_optical_benches, sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_optical_benches; i++) {
        printf("\t\t%d: Testing device 0x%02lX, optical bench feature id 0x%02lX\n", i, deviceID, optical_bench_ids[i]);

        printf("\t\t\tAttempting to get optical bench ID...\n");
        memset(buffer, (int)0, sizeof(buffer));
        sbapi_optical_bench_get_id(deviceID, optical_bench_ids[i], &error, buffer, 79);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        if(0 == error) {
            printf("\t\t\t\tOptical Bench ID: [%s]\n", buffer);
        }
        
        printf("\t\t\tAttempting to get optical bench serial number...\n");
        memset(buffer, (int)0, sizeof(buffer));
        sbapi_optical_bench_get_serial_number(deviceID, optical_bench_ids[i], &error, buffer, 79);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        if(0 == error) {
            printf("\t\t\t\tOptical Bench Serial Number: [%s]\n", buffer);
        }        

        printf("\t\t\tAttempting to get optical bench coating type...\n");
        memset(buffer, (int)0, sizeof(buffer));
        sbapi_optical_bench_get_coating(deviceID, optical_bench_ids[i], &error, buffer, 79);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        if(0 == error) {
            printf("\t\t\t\tOptical Bench coating type: [%s]\n", buffer);
        }

        printf("\t\t\tAttempting to get optical bench filter type...\n");
        memset(buffer, (int)0, sizeof(buffer));
        sbapi_optical_bench_get_filter(deviceID, optical_bench_ids[i], &error, buffer, 79);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        if(0 == error) {
            printf("\t\t\t\tOptical Bench filter type: [%s]\n", buffer);
        } 
        
        printf("\t\t\tAttempting to get optical bench grating type...\n");
        memset(buffer, (int)0, sizeof(buffer));
        sbapi_optical_bench_get_grating(deviceID, optical_bench_ids[i], &error, buffer, 79);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        if(0 == error) {
            printf("\t\t\t\tOptical Bench grating type: [%s]\n", buffer);
        }         
        
        printf("\t\t\tAttempting to get optical bench slit width...\n");
        memset(buffer, (int)0, sizeof(buffer));
        slit_or_fiber=sbapi_optical_bench_get_slit_width_microns(deviceID, optical_bench_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        if(0 == error) {
            printf("\t\t\t\tOptical Bench slit width (microns): [%d]\n", slit_or_fiber);
        } 
        
        printf("\t\t\tAttempting to get optical bench fiber diameter...\n");
        memset(buffer, (int)0, sizeof(buffer));
        slit_or_fiber=sbapi_optical_bench_get_slit_width_microns(deviceID, optical_bench_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        if(0 == error) {
            printf("\t\t\t\tOptical Bench fiber diameter (microns): [%d]\n", slit_or_fiber);
        } 
        
        printf("\t\t%d: Finished testing device 0x%02lX, optical bench feature id 0x%02lX\n", i, deviceID, optical_bench_ids[i]);
    }

    free(optical_bench_ids);

    printf("\tFinished testing optical bench capabilities.\n");
}

void test_spectrometer_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int length;
    long integration_time;
    double max_intensity;
    int number_of_spectrometers;
    long *spectrometer_ids = 0;
    int i;
    double *doubleBuffer = 0;
    unsigned char *byteBuffer = 0;
    int *edarkBuffer = 0;
    int j;

    printf("\n\tTesting spectrometer features:\n");

    printf("\t\tGetting number of spectrometers:\n");
    number_of_spectrometers = sbapi_get_number_of_spectrometer_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_spectrometers,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    if(0 == number_of_spectrometers) {
        printf("\tNo spectrometer capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    spectrometer_ids = (long *)calloc(number_of_spectrometers, sizeof(long));
    printf("\t\tGetting spectrometer feature IDs...\n");
    number_of_spectrometers = sbapi_get_spectrometer_features(deviceID, &error,
            spectrometer_ids, number_of_spectrometers);
    printf("\t\t\tResult is %d [%s]\n", number_of_spectrometers,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for(i = 0; i < number_of_spectrometers; i++) {
        printf("\t\t%d: Testing device 0x%02lX, spectrometer 0x%02lX\n",
                i, deviceID, spectrometer_ids[i]);

        printf("\t\t\tAttempting to set trigger mode to 0\n");
        sbapi_spectrometer_set_trigger_mode(deviceID,
                spectrometer_ids[i], &error, 0);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tGetting maximum intensity\n");
        max_intensity = sbapi_spectrometer_get_maximum_intensity(
                deviceID, spectrometer_ids[i], &error);
        printf("\t\t\t\tResult is %1.2f [%s]\n", max_intensity,
                sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tGetting minimum integration time\n");
        integration_time = sbapi_spectrometer_get_minimum_integration_time_micros(
                deviceID, spectrometer_ids[i], &error);
        printf("\t\t\t\tResult is %ld [%s]\n", integration_time,
                sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tSetting integration time to minimum:\n");
        sbapi_spectrometer_set_integration_time_micros(
                deviceID, spectrometer_ids[i], &error, integration_time);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tGetting spectrum length\n");
        length = sbapi_spectrometer_get_formatted_spectrum_length(deviceID,
                spectrometer_ids[i], &error);
        printf("\t\t\t\tResult is %d [%s]\n", length,
                sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        doubleBuffer = (double *)calloc(length, sizeof(double));
        printf("\t\t\tGetting spectrum...\n");
        length = sbapi_spectrometer_get_formatted_spectrum(deviceID,
                spectrometer_ids[i], &error, doubleBuffer, length);
        printf("\t\t\t\tResult is %d [%s]\n", length,
                sbapi_get_error_string(error));
        printf("\t\t\t\tPixel indices 19 and 20 are: %1.1f, %1.1f\n",
                doubleBuffer[19], doubleBuffer[20]);
        free(doubleBuffer);
        tallyErrors(error, testFailureCount);

        printf("\t\t\tGetting unformatted spectrum length\n");
        length = sbapi_spectrometer_get_unformatted_spectrum_length(deviceID,
                spectrometer_ids[i], &error);
        printf("\t\t\t\tResult is %d [%s]\n", length,
                sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);

        byteBuffer = (unsigned char *)calloc(length, sizeof(unsigned char));
        printf("\t\t\tGetting unformatted spectrum...\n");
        length = sbapi_spectrometer_get_unformatted_spectrum(deviceID,
                spectrometer_ids[i], &error, byteBuffer, length);
        printf("\t\t\t\tResult is %d [%s]\n", length,
                sbapi_get_error_string(error));
        printf("\t\t\t\tByte indices 19 and 20 are: 0x%02X, 0x%02X\n",
                byteBuffer[19], byteBuffer[20]);
        free(byteBuffer);
        tallyErrors(error, testFailureCount);

        doubleBuffer = (double *)calloc(length, sizeof(double));
        printf("\t\t\tGetting wavelengths...\n");
        length = sbapi_spectrometer_get_wavelengths(deviceID,
                spectrometer_ids[i], &error, doubleBuffer, length);
        printf("\t\t\t\tResult is %d [%s]\n", length,
                sbapi_get_error_string(error));
        printf("\t\t\t\tPixel indices 19 and 20 are: %1.2f, %1.2f\n",
                doubleBuffer[19], doubleBuffer[20]);
        free(doubleBuffer);
        tallyErrors(error, testFailureCount);

        printf("\t\t\tGetting electric dark pixel count...\n");
        length = sbapi_spectrometer_get_electric_dark_pixel_count(
                deviceID, spectrometer_ids[i], &error);
        printf("\t\t\t\tResult is %d [%s]\n", length,
                sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        if(length > 0) {
            edarkBuffer = (int *)calloc(length, sizeof(int));
            printf("\t\t\tGetting electric dark pixels...\n");
            length = sbapi_spectrometer_get_electric_dark_pixel_indices(deviceID,
                spectrometer_ids[i], &error, edarkBuffer, length);
            printf("\t\t\t\tResult is %d [%s]\n", length,
                sbapi_get_error_string(error));
            printf("\t\t\tIndices: ");
            tallyErrors(error, testFailureCount);
            
            for(j = 0; j < length; j++) {
                printf("%d ", edarkBuffer[j]);
            }
            printf("\n");
            free(edarkBuffer);
        } else {
            printf("\t\t\t\tSpectrometer has no electric dark pixels.\n");
        }

        
        printf("\t\t%d: Finished testing device 0x%02lX, spectrometer 0x%02lX\n",
                i, deviceID, spectrometer_ids[i]);
    }
    printf("\tFinished testing spectrometer capabilities.\n");

    free(spectrometer_ids);
}


// ToDo Although this tests the shutter firmware, it does not test the shutter hardware
//   This test should be rewritten to look for a change in light to indicate 
//   actual shutter activity instead of a software indicator
void test_shutter_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_shutters;
    long *shutter_ids = 0;
    int i;

    printf("\n\tTesting shutter features:\n");

    printf("\t\tGetting number of shutters:\n");
    number_of_shutters = sbapi_get_number_of_shutter_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_shutters,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    if(0 == number_of_shutters) {
        printf("\tNo shutter capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    shutter_ids = (long *)calloc(number_of_shutters, sizeof(long));
    printf("\t\tGetting shutter feature IDs...\n");
    number_of_shutters = sbapi_get_shutter_features(deviceID, &error,
            shutter_ids, number_of_shutters);
    printf("\t\t\tResult is %d [%s]\n", number_of_shutters,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for(i = 0; i < number_of_shutters; i++) {
        printf("\t\t%d: Testing device 0x%02lX, shutter 0x%02lX\n",
                i, deviceID, shutter_ids[i]);
        printf("\t\t\tNOTE: Some shutter features are synchronized to other\n"
               "\t\t\tNOTE: events, such as spectrometer acquisitions.\n"
               "\t\t\tNOTE: This test only sends the shutter command and this\n"
               "\t\t\tNOTE: may be inadequate to actually toggle the shutter.\n"
               "\t\t\tNOTE: This test is only intended to see if the command\n"
               "\t\t\tNOTE: can be sucessfully created and sent.\n");
        printf("\t\t\tAttempting to close shutter.\n");
        sbapi_shutter_set_shutter_open(deviceID,
                shutter_ids[i], &error, 0);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to open shutter.\n");
        sbapi_shutter_set_shutter_open(deviceID,
                shutter_ids[i], &error, 1);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);

        printf("\t\t%d: Finished testing device 0x%02lX, shutter 0x%02lX\n",
                i, deviceID, shutter_ids[i]);
    }
    free(shutter_ids);

    printf("\tFinished testing shutter capabilities.\n");
}

void test_spectrum_processing_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_sp;
    long *sp_ids = 0;
    int i;
    unsigned char boxcarWidthW, boxcarWidthR, boxcarWidthR2;
    unsigned short int scansToAverageW, scansToAverageR, scansToAverageR2;

    printf("\n\tTesting spectrum processing features:\n");

    printf("\t\tGetting number of spectrum processing features:\n");
    number_of_sp = sbapi_get_number_of_spectrum_processing_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_sp,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_sp) {
        printf("\tNo spectrum processing capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    sp_ids = (long *)calloc(number_of_sp, sizeof(long));
    printf("\t\tGetting spectrum processing feature IDs...\n");
    number_of_sp = sbapi_get_spectrum_processing_features(deviceID, &error,
            sp_ids, number_of_sp);
    printf("\t\t\tResult is %d [%s]\n", number_of_sp, sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_sp; i++) {
        printf("\t\t%d: Testing device 0x%02lX, spectrum processing 0x%02lX\n",
                i, deviceID, sp_ids[i]);
                
        printf("\t\t\tAttempting to read boxcar filter width.\n");
        boxcarWidthR=sbapi_spectrum_processing_boxcar_width_get(deviceID, sp_ids[i], &error);
        printf("\t\t\t\tResult is [%s], boxcar filter width= %d\n", sbapi_get_error_string(error), boxcarWidthR);
        tallyErrors(error, testFailureCount);
        
        if(boxcarWidthR<15)
            boxcarWidthW=boxcarWidthR+1;
        else
            boxcarWidthW=0;
            
        printf("\t\t\tAttempting to write a boxcar filter width of %d.\n", boxcarWidthW);
        sbapi_spectrum_processing_boxcar_width_set(deviceID, sp_ids[i], &error, boxcarWidthW);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tAttempting to read boxcar filter width again.\n");
        boxcarWidthR2=sbapi_spectrum_processing_boxcar_width_get(deviceID, sp_ids[i], &error);
        printf("\t\t\t\tResult is [%s], boxcar filter width = %d\n", sbapi_get_error_string(error), boxcarWidthR2);
        tallyErrors(error, testFailureCount);
        
        if((boxcarWidthR2==boxcarWidthW)&&(boxcarWidthR2!=boxcarWidthR))
            printf("\t\t\t\t...passed...\n");
        else
        {
            printf("\t\t\t\t...failed...\n");
            (*testFailureCount)++;
        }

        
        printf("\t\t\tAttempting to read scans to average.\n");
        scansToAverageR=sbapi_spectrum_processing_scans_to_average_get(deviceID, sp_ids[i], &error);
        printf("\t\t\t\tResult is [%s], scans to average = %d\n", sbapi_get_error_string(error), scansToAverageR);
        tallyErrors(error, testFailureCount);
        
        if((scansToAverageR>=1) && (scansToAverageR<15))
            scansToAverageW=scansToAverageR+1;
        else
            scansToAverageW=1;
            
        printf("\t\t\tAttempting to write a scans to average number of %d.\n", scansToAverageW);
        sbapi_spectrum_processing_scans_to_average_set(deviceID, sp_ids[i], &error, scansToAverageW);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tAttempting to read scans to average again.\n");
        scansToAverageR2=sbapi_spectrum_processing_scans_to_average_get(deviceID, sp_ids[i], &error);
        printf("\t\t\t\tResult is [%s], scans to average = %d\n", sbapi_get_error_string(error), scansToAverageR2);
        tallyErrors(error, testFailureCount);
        
        if((scansToAverageR2==scansToAverageW)&&(scansToAverageR2!=scansToAverageR))
            printf("\t\t\t\t...passed...\n");
        else
        {
            printf("\t\t\t\t...failed...\n");
            (*testFailureCount)++;
        }

        printf("\t\t%d: Finished testing device 0x%02lX, spectrum processing feature 0x%02lX\n",
                i, deviceID, sp_ids[i]);
    }
    free(sp_ids);

    printf("\tFinished testing spectrum processing capabilities.\n");
}

void test_light_source_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_light_sources;
    long *light_source_ids = 0;
    int source_count;
    int i;
    int source;
    unsigned char flag;
    float intensity = 0;

    printf("\n\tTesting light source features:\n");

    printf("\t\tGetting number of light sources:\n");
    number_of_light_sources = sbapi_get_number_of_light_source_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_light_sources,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_light_sources) {
        printf("\tNo light source capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    light_source_ids = (long *)calloc(number_of_light_sources, sizeof(long));
    printf("\t\tGetting light source feature IDs...\n");
    number_of_light_sources = sbapi_get_light_source_features(deviceID, &error,
            light_source_ids, number_of_light_sources);
    printf("\t\t\tResult is %d [%s]\n", number_of_light_sources,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_light_sources; i++) {
        printf("\t\t%d: Testing device 0x%02lX, light source 0x%02lX\n",
                i, deviceID, light_source_ids[i]);

        printf("\t\t\tAttempting to get light source count for feature.\n");
        source_count = sbapi_light_source_get_count(deviceID,
                light_source_ids[i], &error);
        printf("\t\t\t\tResult is %d [%s]\n", source_count,
                sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        for(source = 0; source < source_count; source++) {
            printf("\t\t\tExamining light source %d\n", source);

            printf("\t\t\t\tAttempting to query whether intensity control is available\n");
            flag = sbapi_light_source_has_variable_intensity(deviceID, light_source_ids[i],
                        &error, source);
            printf("\t\t\t\tResult is %s [%s]\n", 0 == flag ? "false" : "true",
                sbapi_get_error_string(error));
            tallyErrors(error, testFailureCount);
            
            if(0 != flag) {
                printf("\t\t\t\tAttempting to get light source %d percent intensity\n", source);
                intensity = sbapi_light_source_get_intensity(deviceID, light_source_ids[i],
                        &error, source);
                printf("\t\t\t\t\tResult is %1.1f%% [%s]\n", intensity * 100.0,
                    sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
                
                printf("\t\t\t\tAttempting to set intensity to half of maximum\n");
                sbapi_light_source_set_intensity(deviceID, light_source_ids[i],
                        &error, source, 0.5);
                printf("\t\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
                
                /* Re-read the intensity to see if it was applied */
                printf("\t\t\t\tAttempting to get light source %d percent intensity\n", source);
                intensity = sbapi_light_source_get_intensity(deviceID, light_source_ids[i],
                        &error, source);
                printf("\t\t\t\t\tResult is %1.1f%% [%s]\n", intensity * 100.0,
                    sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
            }

            printf("\t\t\t\tAttempting to query whether enable is available\n");
            flag = sbapi_light_source_has_enable(deviceID, light_source_ids[i],
                        &error, source);
            printf("\t\t\t\tResult is %s [%s]\n", 0 == flag ? "false" : "true",
                sbapi_get_error_string(error));
            tallyErrors(error, testFailureCount);
            
            if(0 != flag) {
                printf("\t\t\t\tAttempting to get light source %d enable status\n", source);
                flag = sbapi_light_source_is_enabled(deviceID, light_source_ids[i],
                        &error, source);
                printf("\t\t\t\t\tResult is %s [%s]\n", 0 == flag ? "false" : "true",
                    sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
                
                printf("\t\t\t\tAttempting to enable light source %d\n", source);
                sbapi_light_source_set_enable(deviceID, light_source_ids[i],
                        &error, source, 1);
                printf("\t\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
                
                printf("\t\t\t\tAttempting to get light source %d enable status\n", source);
                flag = sbapi_light_source_is_enabled(deviceID, light_source_ids[i],
                        &error, source);
                printf("\t\t\t\t\tResult is %s [%s]\n", 0 == flag ? "false" : "true",
                    sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
                
                printf("\t\t\t\tAttempting to disable light source %d\n", source);
                sbapi_light_source_set_enable(deviceID, light_source_ids[i],
                        &error, source, 0);
                printf("\t\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
                
                printf("\t\t\t\tAttempting to get light source %d enable status\n", source);
                flag = sbapi_light_source_is_enabled(deviceID, light_source_ids[i],
                        &error, source);
                printf("\t\t\t\t\tResult is %s [%s]\n", 0 == flag ? "false" : "true",
                    sbapi_get_error_string(error));
                tallyErrors(error, testFailureCount);
                
            }
        }
    }

    free(light_source_ids);
    printf("\tFinished testing light source capabilities.\n");
}

void test_lamp_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_lamps;
    long *lamp_ids = 0;
    int i;

    printf("\n\tTesting lamp features:\n");

    printf("\t\tGetting number of lamps:\n");
    number_of_lamps = sbapi_get_number_of_lamp_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_lamps,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_lamps) {
        printf("\tNo lamp capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    lamp_ids = (long *)calloc(number_of_lamps, sizeof(long));
    printf("\t\tGetting lamp feature IDs...\n");
    number_of_lamps = sbapi_get_lamp_features(deviceID, &error,
            lamp_ids, number_of_lamps);
    printf("\t\t\tResult is %d [%s]\n", number_of_lamps,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_lamps; i++) {
        printf("\t\t%d: Testing device 0x%02lX, lamp 0x%02lX\n",
                i, deviceID, lamp_ids[i]);
        printf("\t\t\tNOTE: Some lamp features are synchronized to other\n"
               "\t\t\tNOTE: events, such as spectrometer acquisitions.\n"
               "\t\t\tNOTE: This test only sends the lamp command and this\n"
               "\t\t\tNOTE: may be inadequate to actually toggle the lamp.\n"
               "\t\t\tNOTE: This test is only intended to see if the command\n"
               "\t\t\tNOTE: can be sucessfully created and sent.\n");
        printf("\t\t\tAttempting to enable lamp.\n");
        sbapi_lamp_set_lamp_enable(deviceID,
                lamp_ids[i], &error, 1);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tAttempting to disable lamp.\n");
        sbapi_lamp_set_lamp_enable(deviceID,
                lamp_ids[i], &error, 0);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));

        printf("\t\t%d: Finished testing device 0x%02lX, lamp 0x%02lX\n",
                i, deviceID, lamp_ids[i]);
        tallyErrors(error, testFailureCount);
    }
    free(lamp_ids);
    printf("\tFinished testing lamp capabilities.\n");
}

void test_eeprom_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_eeproms;
    long *eeprom_ids = 0;
    unsigned char buffer[80];
    int i;

    printf("\n\tTesting EEPROM features:\n");

    printf("\t\tGetting number of EEPROMs:\n");
    number_of_eeproms = sbapi_get_number_of_eeprom_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_eeproms,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_eeproms) {
        printf("\tNo EEPROM capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    eeprom_ids = (long *)calloc(number_of_eeproms, sizeof(long));
    printf("\t\tGetting EEPROM feature IDs...\n");
    number_of_eeproms = sbapi_get_eeprom_features(deviceID, &error,
            eeprom_ids, number_of_eeproms);
    printf("\t\t\tResult is %d [%s]\n", number_of_eeproms,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_eeproms; i++) {
        printf("\t\t%d: Testing device 0x%02lX, eeprom 0x%02lX\n",
                i, deviceID, eeprom_ids[i]);

        printf("\t\t\tAttempting to get EEPROM slot 0...\n");
        memset(buffer, (int)0, sizeof(buffer));
        sbapi_eeprom_read_slot(deviceID, eeprom_ids[i], &error, 0, buffer, 79);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        if(0 == error) {
            printf("\t\t\t\tEEPROM slot 0: [%s]\n", buffer);
        }

        printf("\t\t%d: Finished testing device 0x%02lX, eeprom 0x%02lX\n",
                i, deviceID, eeprom_ids[i]);
    }
    free(eeprom_ids);
    printf("\tFinished testing EEPROM capabilities.\n");
}


void test_irradcal_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_irradcals;
    long *irradcal_ids = 0;
    float buffer[20];
    int i;
    int length;
    int flag;
    float area;

    printf("\n\tTesting irradiance calibration features:\n");

    printf("\t\tGetting number of irradiance calibration features:\n");
    number_of_irradcals = sbapi_get_number_of_irrad_cal_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_irradcals,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_irradcals) {
        printf("\tNo irradiance calibration capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    irradcal_ids = (long *)calloc(number_of_irradcals, sizeof(long));
    printf("\t\tGetting irradiance calibration feature IDs...\n");
    number_of_irradcals = sbapi_get_irrad_cal_features(deviceID, &error,
            irradcal_ids, number_of_irradcals);
    printf("\t\t\tResult is %d [%s]\n", number_of_irradcals,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for(i = 0; i < number_of_irradcals; i++) {
        printf("\t\t%d: Testing device 0x%02lX, irradcal 0x%02lX\n",
                i, deviceID, irradcal_ids[i]);

        // Note: this test can fail on OBP devices (STS) if no irradiance calibration
        //       has yet been written to the device (per STS Data Sheet, p26)
        printf("\t\t\tAttempting to get partial irradiance calibration...\n");
        memset(buffer, (int)0, sizeof(buffer));
        length = sbapi_irrad_calibration_read(deviceID, irradcal_ids[i], &error,
            buffer, 20);
        printf("\t\t\t\tRead %d values [%s]\n", length, sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        if(0 == error && length > 0) {
            printf("\t\t\t\tFirst calibration term: %1.2e\n", buffer[0]);
        }

        printf("\t\t\tAttempting to check for collection area...\n");
        flag = sbapi_irrad_calibration_has_collection_area(deviceID,
                irradcal_ids[i], &error);
        printf("\t\t\t\tResult is %d [%s]\n", flag, sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        if(0 != flag) {
            printf("\t\t\tAttempting to read collection area...\n");
            area = sbapi_irrad_calibration_read_collection_area(deviceID,
                irradcal_ids[i], &error);
            printf("\t\t\t\tResult is %1.2e [%s]\n", area, sbapi_get_error_string(error));
            tallyErrors(error, testFailureCount);
        } else {
            printf("\t\t\tNo collection area available.\n");
        }

        printf("\t\t%d: Finished testing device 0x%02lX, irradcal 0x%02lX\n",
                i, deviceID, irradcal_ids[i]);
    }
    free(irradcal_ids);

    printf("\tFinished testing irradiance calibration capabilities.\n");
}

void test_tec_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_tec;
    long *tec_ids = 0;
    int i;
    float temperature;

    printf("\n\tTesting TEC features:\n");

    printf("\t\tGetting number of TECs:\n");
    number_of_tec = sbapi_get_number_of_thermo_electric_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_tec,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_tec) {
        printf("\tNo TEC capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    tec_ids = (long *)calloc(number_of_tec, sizeof(long));
    printf("\t\tGetting TEC feature IDs...\n");
    number_of_tec = sbapi_get_thermo_electric_features(deviceID, &error,
            tec_ids, number_of_tec);
    printf("\t\t\tResult is %d [%s]\n", number_of_tec,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_tec; i++) {
        printf("\t\t%d: Testing device 0x%02lX, tec 0x%02lX\n",
                i, deviceID, tec_ids[i]);

        printf("\t\t\tAttempting to read TEC temperature...\n");
        temperature = (float)sbapi_tec_read_temperature_degrees_C(deviceID,
                            tec_ids[i], &error);
        printf("\t\t\t\tResult is %1.2f [%s]\n", temperature,
                            sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tAttempting to enable TEC...\n");
        sbapi_tec_set_enable(deviceID, tec_ids[i], &error, 1);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tAttempting to set TEC temperature to -5C...\n");
        sbapi_tec_set_temperature_setpoint_degrees_C(deviceID,
                            tec_ids[i], &error, -5.0);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t%d: Finished testing device 0x%02lX, tec 0x%02lX\n",
                i, deviceID, tec_ids[i]);
    }

    free(tec_ids);
    printf("\tFinished testing TEC capabilities.\n");
}

void test_nonlinearity_coeffs_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_nonlinearity_coeff_features;
    long *nonlinearity_coeff_feature_ids = 0;
    double buffer[10];
    int i;
    int length;

    printf("\n\tTesting nonlinearity coefficient features:\n");

    printf("\t\tGetting number of nonlinearity coefficient features:\n");
    number_of_nonlinearity_coeff_features =
        sbapi_get_number_of_nonlinearity_coeffs_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_nonlinearity_coeff_features,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_nonlinearity_coeff_features) {
        printf("\tNo nonlinearity coefficient capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    nonlinearity_coeff_feature_ids =
        (long *)calloc(number_of_nonlinearity_coeff_features, sizeof(long));
    printf("\t\tGetting nonlinearity coefficient feature IDs...\n");
    number_of_nonlinearity_coeff_features = sbapi_get_nonlinearity_coeffs_features(
            deviceID, &error, nonlinearity_coeff_feature_ids,
            number_of_nonlinearity_coeff_features);
    printf("\t\t\tResult is %d [%s]\n", number_of_nonlinearity_coeff_features,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_nonlinearity_coeff_features; i++) {
        printf("\t\t%d: Testing device 0x%02lX, nonlinearity coeffs 0x%02lX\n",
                i, deviceID, nonlinearity_coeff_feature_ids[i]);

        printf("\t\t\tAttempting to get nonlinearity coefficients...\n");
        memset(buffer, (int)0, sizeof(buffer));
        length = sbapi_nonlinearity_coeffs_get(deviceID,
            nonlinearity_coeff_feature_ids[i], &error, buffer, 10);
        printf("\t\t\t\tResult is %d [%s]\n", length, sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        if(0 == error && length > 0) {
            printf("\t\t\t\tFirst calibration term: %1.2e\n", buffer[0]);
        }

        printf("\t\t%d: Finished testing device 0x%02lX, nonlinearity coeffs 0x%02lX\n",
                i, deviceID, nonlinearity_coeff_feature_ids[i]);
    }
    free(nonlinearity_coeff_feature_ids);

    printf("\tFinished testing nonlinearity coefficient capabilities.\n");
}


void test_temperature_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_temperature_features;
    long *temperature_feature_ids = 0;
    double buffer[10];  // how many temperatures could there be on _any_ given spectrometer
    int i;
    int t_index;
    int length;
    float myTemp;

    printf("\n\tTesting temperature features:\n");

    printf("\t\tGetting number of temperature features:\n");
    number_of_temperature_features =
        sbapi_get_number_of_temperature_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_temperature_features,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_temperature_features) {
        printf("\tNo temperature capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    temperature_feature_ids =
        (long *)calloc(number_of_temperature_features, sizeof(long));
    printf("\t\tGetting temperature feature IDs...\n");
    number_of_temperature_features = sbapi_get_temperature_features(
            deviceID, &error, temperature_feature_ids,
            number_of_temperature_features);
    printf("\t\t\tResult is %d [%s]\n", number_of_temperature_features,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_temperature_features; i++) {
        printf("\t\t%d: Testing device 0x%02lX, temperatures 0x%02lX\n",
                i, deviceID, temperature_feature_ids[i]);

        printf("\t\t\tAttempting to get all temperatures...\n");
        memset(buffer, (int)0, sizeof(buffer));        
        length = sbapi_temperature_get_all(deviceID,
            temperature_feature_ids[i], &error, buffer, 10);
        printf("\t\t\t\tResult is %d [%s]\n", length, sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        for(t_index=0; t_index<length; t_index++) {
            if(0 == error && length > 0) {
                printf("\t\t\t\tTemperature(%d): %2.2f\n", t_index, buffer[t_index]);
            }
        }
        
        printf("\t\t\tAttempting to get the number of specific temperatures...\n");
        
        length = sbapi_temperature_count_get(deviceID,
            temperature_feature_ids[i], &error);
        printf("\t\t\t\tResult is %d [%s]\n", length, sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        for(t_index=0; t_index<length; t_index++) {
            if(0 == error && length > 0) {
                myTemp=sbapi_temperature_get(deviceID, temperature_feature_ids[i], &error, t_index);
                printf("\t\t\t\tTemperature(%d): %2.2f\n", t_index, myTemp);
                tallyErrors(error, testFailureCount);
            }
        }
        
        printf("\t\t%d: Finished testing device 0x%02lX, temperatures 0x%02lX\n",
                i, deviceID, temperature_feature_ids[i]);
    }
    free(temperature_feature_ids);

    printf("\tFinished testing temperature capabilities.\n");
}


void test_stray_light_coeffs_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_stray_light_coeff_features;
    long *stray_light_coeff_feature_ids = 0;
    double buffer[5];
    int i;
    int length;

    printf("\n\tTesting stray light coefficient features:\n");

    printf("\t\tGetting number of stray light coefficient features:\n");
    number_of_stray_light_coeff_features = sbapi_get_number_of_stray_light_coeffs_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_stray_light_coeff_features,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_stray_light_coeff_features) {
        printf("\tNo stray light coefficient capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    stray_light_coeff_feature_ids =
        (long *)calloc(number_of_stray_light_coeff_features, sizeof(long));
    printf("\t\tGetting stray light coefficient feature IDs...\n");
    number_of_stray_light_coeff_features = sbapi_get_stray_light_coeffs_features(
            deviceID, &error, stray_light_coeff_feature_ids,
            number_of_stray_light_coeff_features);
    printf("\t\t\tResult is %d [%s]\n", number_of_stray_light_coeff_features,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_stray_light_coeff_features; i++) {
        printf("\t\t%d: Testing device 0x%02lX, stray light coeffs 0x%02lX\n",
                i, deviceID, stray_light_coeff_feature_ids[i]);

        printf("\t\t\tAttempting to get stray light coefficients...\n");
        memset(buffer, (int)0, sizeof(buffer));
        length = sbapi_stray_light_coeffs_get(deviceID,
            stray_light_coeff_feature_ids[i], &error, buffer, 5);
        printf("\t\t\t\tResult is %d [%s]\n", length, sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        if(0 == error && length > 0) {
            printf("\t\t\t\tFirst calibration term: %1.2e\n", buffer[0]);
        }

        printf("\t\t%d: Finished testing device 0x%02lX, stray light coeffs 0x%02lX\n",
                i, deviceID, stray_light_coeff_feature_ids[i]);
    }
    free(stray_light_coeff_feature_ids);

    printf("\tFinished testing stray light coefficient capabilities.\n");
}


void test_continuous_strobe_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_cont_strobes;
    long *cont_strobe_ids = 0;
    int i;

    printf("\n\tTesting continuous strobe features:\n");

    printf("\t\tGetting number of continuous strobes:\n");
    number_of_cont_strobes = sbapi_get_number_of_continuous_strobe_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_cont_strobes,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    if(0 == number_of_cont_strobes) {
        printf("\tNo continuous strobe capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    cont_strobe_ids = (long *)calloc(number_of_cont_strobes, sizeof(long));
    printf("\t\tGetting continuous strobe feature IDs...\n");
    number_of_cont_strobes = sbapi_get_continuous_strobe_features(deviceID, &error,
            cont_strobe_ids, number_of_cont_strobes);
    printf("\t\t\tResult is %d [%s]\n", number_of_cont_strobes,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);
    
    for(i = 0; i < number_of_cont_strobes; i++) {
        printf("\t\t%d: Testing device 0x%02lX, continuous strobe 0x%02lX\n",
                i, deviceID, cont_strobe_ids[i]);
        printf("\t\t\tAttempting to enable continous strobe.\n");
        sbapi_continuous_strobe_set_continuous_strobe_enable(deviceID,
                cont_strobe_ids[i], &error, 1);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tAttempting to set period to 20ms.\n");
        sbapi_continuous_strobe_set_continuous_strobe_period_micros(deviceID,
                cont_strobe_ids[i], &error, 20000);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tDelaying to allow verification.\n");
#ifndef _WINDOWS
        sleep(2);
#else
        Sleep(2000);
#endif

        printf("\t\t\tAttempting to set period to 50ms.\n");
        sbapi_continuous_strobe_set_continuous_strobe_period_micros(deviceID,
                cont_strobe_ids[i], &error, 50000);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tDelaying to allow verification.\n");
#ifndef _WINDOWS
        sleep(2);
#else
        Sleep(2000);
#endif

        printf("\t\t\tAttempting to disable continous strobe.\n");
        sbapi_continuous_strobe_set_continuous_strobe_enable(deviceID,
                cont_strobe_ids[i], &error, 0);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);

        printf("\t\t%d: Finished testing device 0x%02lX, continuous strobe 0x%02lX\n",
                i, deviceID, cont_strobe_ids[i]);
    }
    free(cont_strobe_ids);

    printf("\tFinished testing continuous strobe capabilities.\n");
}


void test_data_buffer_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_data_buffers = 0;
    long *data_buffer_ids = 0;
    int i;
    unsigned long capacity = 0;
    unsigned long minCapacity = 0;
    unsigned long maxCapacity = 0;
    unsigned long oldCapacity = 0;
    unsigned long targetCapacity = 0;
    unsigned long count = 0;
    unsigned long newCount = 0;

    printf("\n\tTesting data buffer features:\n");

    printf("\t\tGetting number of data buffer features:\n");
    number_of_data_buffers = sbapi_get_number_of_data_buffer_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_data_buffers,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    if(0 == number_of_data_buffers) {
        printf("\tNo data buffer capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    data_buffer_ids = (long *)calloc(number_of_data_buffers, sizeof(long));
    printf("\t\tGetting data buffer feature IDs...\n");
    number_of_data_buffers = sbapi_get_data_buffer_features(deviceID, &error,
            data_buffer_ids, number_of_data_buffers);
    printf("\t\t\tResult is %d [%s]\n", number_of_data_buffers,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for(i = 0; i < number_of_data_buffers; i++) {
        printf("\t\t%d: Testing device 0x%02lX, data buffer 0x%02lX\n",
                i, deviceID, data_buffer_ids[i]);

        printf("\t\t\tAttempting to get minimum buffer size...\n");
        minCapacity = sbapi_data_buffer_get_buffer_capacity_minimum(deviceID, data_buffer_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tMinimum buffer size: %ld\n", minCapacity);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get maximum buffer size...\n");
        maxCapacity = sbapi_data_buffer_get_buffer_capacity_maximum(deviceID, data_buffer_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tMaximum buffer size: %ld\n", maxCapacity);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get current buffer size...\n");
        oldCapacity = sbapi_data_buffer_get_buffer_capacity(deviceID, data_buffer_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tCurrent buffer size: %ld\n", oldCapacity);
        }
        tallyErrors(error, testFailureCount);

        if(oldCapacity != maxCapacity) {
            targetCapacity = maxCapacity;
        } else if(maxCapacity > minCapacity) {
            /* The current capacity setting is the maximum, so try to back off by one.
             * This test confirms that there is at least some room between min and max.
             */
            targetCapacity = maxCapacity - 1;
        } else {
            /* Not possible to change the capacity to anything new */
            targetCapacity = oldCapacity;
        }

        if(targetCapacity != oldCapacity) {
            printf("\t\t\tAttempting to set buffer size to %ld...\n", targetCapacity);
            sbapi_data_buffer_set_buffer_capacity(deviceID, data_buffer_ids[i], &error, targetCapacity);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            tallyErrors(error, testFailureCount);

            printf("\t\t\tAttempting to get current buffer size...\n");
            capacity = sbapi_data_buffer_get_buffer_capacity(deviceID, data_buffer_ids[i], &error);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            if(0 == error) {
                printf("\t\t\t\tCurrent buffer size: %ld\n", capacity);
            }
            tallyErrors(error, testFailureCount);
            if(capacity != targetCapacity) {
                printf("\t\t\t\tERROR: did not get back the expected value.\n");
                (*testFailureCount)++;
            }
        }

        printf("\t\t\tAttempting to get number of items in the buffer...\n");
        count = sbapi_data_buffer_get_number_of_elements(deviceID, data_buffer_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tNumber of items ready for retrieval: %ld\n", count);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tWaiting to allow more data to be buffered...\n");
        /* This assumes that the spectrometer test occurred before this, so the
         * integration time and trigger mode were set in a way that this will
         * keep acquiring a few spectra.
         */
#ifndef _WINDOWS
        sleep(2);
#else
        Sleep(2000);
#endif

        printf("\t\t\tAttempting to get number of items in the buffer...\n");
        count = sbapi_data_buffer_get_number_of_elements(deviceID, data_buffer_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tNumber of items ready for retrieval: %ld\n", count);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to clear buffer...\n");
        sbapi_data_buffer_clear(deviceID, data_buffer_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get number of items in the buffer...\n");
        newCount = sbapi_data_buffer_get_number_of_elements(deviceID, data_buffer_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tNumber of items ready for retrieval: %ld\n", newCount);
        }
        tallyErrors(error, testFailureCount);

        if(newCount >= count && newCount > 1) {
            /* It is expected that the count would drop to zero, but there is a chance that
             * another scan could be stored between the clear and the query.  This should
             * allow at most one scan in that interval without throwing the error.
             */
            printf("\t\t\t\tERROR: count did not seem to drop when the buffer was cleared.\n");
            (*testFailureCount)++;
        }

        if(targetCapacity != oldCapacity) {
            printf("\t\t\tAttempting to set buffer size back to %ld...\n", oldCapacity);
            sbapi_data_buffer_set_buffer_capacity(deviceID, data_buffer_ids[i], &error, oldCapacity);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            tallyErrors(error, testFailureCount);

            printf("\t\t\tAttempting to get current buffer size...\n");
            capacity = sbapi_data_buffer_get_buffer_capacity(deviceID, data_buffer_ids[i], &error);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            if(0 == error) {
                printf("\t\t\t\tCurrent buffer size: %ld\n", capacity);
            }
            if(capacity != oldCapacity) {
                printf("\t\t\t\tERROR: did not get back the expected value.\n");
                (*testFailureCount)++;
            }
            tallyErrors(error, testFailureCount);
        }
        
        printf("\t\t%d: Finished testing device 0x%02lX, data buffer 0x%02lX\n",
                i, deviceID, data_buffer_ids[i]);
    }
    free(data_buffer_ids);

    printf("\tFinished testing data buffer capabilities.\n");
}

void test_acquisition_delay_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {
    int error = 0;
    int number_of_acq_delays = 0;
    long *acq_delay_ids = 0;
    int i;
    unsigned long delay_min;
    unsigned long delay_max;
    unsigned long delay_inc;
    unsigned long old_delay;
    unsigned long new_delay;
    unsigned long temp;

    printf("\n\tTesting acquisition delay features:\n");

    printf("\t\tGetting number of acquisition delay features:\n");
    number_of_acq_delays = sbapi_get_number_of_acquisition_delay_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_acq_delays,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    if(0 == number_of_acq_delays) {
        printf("\tNo acquisition delay capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    acq_delay_ids = (long *)calloc(number_of_acq_delays, sizeof(long));
    printf("\t\tGetting acquisition delay feature IDs...\n");
    number_of_acq_delays = sbapi_get_acquisition_delay_features(deviceID, &error,
            acq_delay_ids, number_of_acq_delays);
    printf("\t\t\tResult is %d [%s]\n", number_of_acq_delays,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for(i = 0; i < number_of_acq_delays; i++) {
        printf("\t\t%d: Testing device 0x%02lX, acquisition delay 0x%02lX\n",
                i, deviceID, acq_delay_ids[i]);

        printf("\t\t\tAttempting to get minimum delay...\n");
        delay_min = sbapi_acquisition_delay_get_delay_minimum_microseconds(deviceID, acq_delay_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tMinimum acquisition delay: %ld usec\n", delay_min);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get maximum delay...\n");
        delay_max = sbapi_acquisition_delay_get_delay_maximum_microseconds(deviceID, acq_delay_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tMaximum acquisition delay: %ld usec\n", delay_max);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get delay increment...\n");
        delay_inc = sbapi_acquisition_delay_get_delay_increment_microseconds(deviceID, acq_delay_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tAcquisition delay increment: %ld usec\n", delay_inc);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get current acquisition delay...\n");
        old_delay = sbapi_acquisition_delay_get_delay_microseconds(deviceID, acq_delay_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tAcquisition delay: %ld usec\n", old_delay);
        } else {
            printf("\t\t\t\tAcquisition delay read may cause an error if never previously written; moving on\n");
        }

        if(old_delay != delay_max) {
            new_delay = delay_max;
        } else if(delay_min < delay_max) {
            new_delay = delay_max - delay_inc;
        } else {
            new_delay = old_delay;
        }

        printf("\t\t\tAttempting to set acquisition delay to %ld usec...\n", new_delay);
        sbapi_acquisition_delay_set_delay_microseconds(deviceID, acq_delay_ids[i], &error, new_delay);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get current acquisition delay...\n");
        temp = sbapi_acquisition_delay_get_delay_microseconds(deviceID, acq_delay_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tAcquisition delay: %ld usec\n", temp);
        }
        tallyErrors(error, testFailureCount);
        
        printf("\t\t\tAttempting to set acquisition delay back to %ld usec...\n", old_delay);
        sbapi_acquisition_delay_set_delay_microseconds(deviceID, acq_delay_ids[i], &error, old_delay);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to get current acquisition delay...\n");
        temp = sbapi_acquisition_delay_get_delay_microseconds(deviceID, acq_delay_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tAcquisition delay: %ld usec\n", temp);
        }
        tallyErrors(error, testFailureCount);
    }
    free(acq_delay_ids);

    printf("\tFinished testing acquisition delay capabilities\n");
}

void test_pixel_binning_feature(long deviceID, int *unsupportedFeatureCount, int *testFailureCount) {

    int error = 0;
    int number_of_pixel_binning;
    long *pixel_binning_ids;
    int i;
    unsigned char max_binning;
    unsigned char factory_default_pixel_binning;
    unsigned char factor;
    unsigned char f;
    unsigned char check_default;

    printf("\n\tTesting pixel binning features:\n");

    printf("\t\tGetting number of pixel binning features:\n");
    number_of_pixel_binning = sbapi_get_number_of_pixel_binning_features(deviceID, &error);
    printf("\t\t\tResult is %d [%s]\n", number_of_pixel_binning,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    if (0 == number_of_pixel_binning) {
        printf("\tNo pixel binning capabilities found.\n");
                tallyUnsupportedFeatures(unsupportedFeatureCount);

        return;
    }

    pixel_binning_ids = (long *)calloc(number_of_pixel_binning, sizeof(long));
    printf("\t\tGetting acquisition delay feature IDs...\n");
    number_of_pixel_binning = sbapi_get_pixel_binning_features(deviceID, &error,
            pixel_binning_ids, number_of_pixel_binning);
    printf("\t\t\tResult is %d [%s]\n", number_of_pixel_binning,
            sbapi_get_error_string(error));
    tallyErrors(error, testFailureCount);

    for (i = 0; i < number_of_pixel_binning; i++) {
        printf("\t\t\tAttempting to get maximum pixel binning factor...\n");
        max_binning = sbapi_binning_get_max_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if(0 == error) {
            printf("\t\t\t\tMaximum pixel binning: %d\n", max_binning);
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tAttempting to set pixel binning factor too high...\n");
        sbapi_binning_set_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error, max_binning + 1);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if (0 == error) {
            printf("\t\t\t\tERROR: set the pixel binning too high.\n");
            ++testFailureCount;
        }
        error = 0;
        tallyErrors(error, testFailureCount);

        /* Remember the default pixel binning for the end of the test. We are
         * assuming that the spectrometer has the factory default pixel binning at this point.
         */
        factory_default_pixel_binning = sbapi_binning_get_default_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error);

        for (factor = max_binning; factor <= max_binning && factor >= 0; --factor) {
            printf("\t\t\tAttempting to set pixel binning factor...\n");
            sbapi_binning_set_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error, factor);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            if (0 == error) {
                printf("\t\t\t\tPixel binning set to: %d\n", factor);
            }
            tallyErrors(error, testFailureCount);

            // TODO check that the number of wavelengths and intensities returned are adjusted

            printf("\t\t\tAttempting to get pixel binning factor...\n");
            f = sbapi_binning_get_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            if (0 == error) {
                printf("\t\t\t\tPixel binning returned: %d\n", f);
            }
            if (f != factor) {
                printf("\t\t\t\tERROR: pixel binning factor returned does not match the set value.\n");
                ++testFailureCount;
            }
            tallyErrors(error, testFailureCount);

            printf("\t\t\tAttempting to set the default pixel binning factor...\n");
            sbapi_binning_set_default_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error, factor);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            if (0 == error) {
                printf("\t\t\t\tDefault pixel binning set to: %d\n", factor);
            }
            tallyErrors(error, testFailureCount);

            printf("\t\t\tAttempting to get the default pixel binning factor...\n");
            f = sbapi_binning_get_default_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error);
            printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
            if (0 == error) {
                printf("\t\t\t\tDefault pixel binning returned: %d\n", f);
            }
            if (f != factor) {
                printf("\t\t\t\tERROR: default pixel binning factor returned does not match the set value.\n");
                ++testFailureCount;
            }
            tallyErrors(error, testFailureCount);
        }
        printf("\t\t\tAttempting to reset the factory default pixel binning factor...\n");
        sbapi_binning_reset_default_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if (0 == error) {
            printf("\t\t\t\tReset the default pixel binning factor\n\n");
        }
        tallyErrors(error, testFailureCount);

        printf("\t\t\tChecking the factory default pixel binning factor...\n");
        check_default = sbapi_binning_get_default_pixel_binning_factor(deviceID, pixel_binning_ids[i], &error);
        printf("\t\t\t\tResult is [%s]\n", sbapi_get_error_string(error));
        if (0 == error) {
            printf("\t\t\t\tGet the default pixel binning factor: %d\n\n", check_default);
        }
        tallyErrors(error, testFailureCount);
        if (check_default != factory_default_pixel_binning) {
            printf("\t\t\t\tERROR: default pixel binning factor returned does not match the factory value.\n");
            ++testFailureCount;
        }
        tallyErrors(error, testFailureCount);
    }
    free(pixel_binning_ids);

    printf("\tFinished testing pixel binning capabilities\n");
}
