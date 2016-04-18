/*******************************************************
 * File:    seabreeze_test_linux.c
 * Date:    July 2009
 * Author:  Ocean Optics, Inc.
 *
 * This is a test program to exercise some of the
 * SeaBreeze functionality.  This will not compile
 * under Windows, so if you are using Visual Studio,
 * remove this file from the project.
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
#include <sys/time.h>
#include <unistd.h>
#include "api/SeaBreezeWrapper.h"

void speed_test(int index);
void get_spectrum_test(int index);
void get_raw_spectrum_test(int index);
void get_wavelengths_test(int index);
void read_eeprom_test(int index);
void test_corrections(int index);
void strobe_enable_test(int index);
void trigger_mode_test(int index);
void tec_test(int index);
void test_irrad_cal(int index);
void read_serial_number_test(int index);
void shutter_test(int index);
void light_source_test(int index);
void data_buffer_test(int index);

/* Prototypes from spectral_correction.c */
int is_electric_dark_supported(int index, int *error_code);
int get_edark_corrected_spectrum(int index, int *error_code, double *buffer,
        int buffer_length);
int do_nonlinearity_correction(int index, int *error_code,
        double *pixels, double pixel_count);

const char* get_error_string(int error) {
    static char buffer[32];
    seabreeze_get_error_string(error, buffer, sizeof(buffer));
    return buffer;
}

int main() {
    int flag;
    int error;
    char type[16];
    int device_count = 0;
    int i;

    for(i = 0; i < SEABREEZE_MAX_DEVICES; i++) {
        printf("\nOpening spectrometer %d.\n", i);
        flag = seabreeze_open_spectrometer(i, &error);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        if(0 == flag) {
            device_count++;
        } else {
            break;
        }
    }

    for(i = 0; i < device_count; i++) {
        printf("\nGetting device %d name.\n", i);
        seabreeze_get_model(i, &error, type, sizeof(type));
        printf("...Result is (%s) [%s]\n", type, get_error_string(error));
    }

    for(i = 0; i < device_count; i++) {
        printf("\nSetting device %d integration time to 100ms.\n", i);
        seabreeze_set_integration_time_microsec(i, &error, 100000);
        printf("...Result is [%s]\n", get_error_string(error));
    }

    for(i = 0; i < device_count; i++) {
        printf("\n\nBeginning test suite for device %d\n", i);
        read_serial_number_test(i);  // done twice to workaround lockup bug FIXME
        read_serial_number_test(i);
        tec_test(i);
        strobe_enable_test(i);
        trigger_mode_test(i);
        read_eeprom_test(i);
        test_irrad_cal(i);
        get_wavelengths_test(i);
        get_spectrum_test(i);
        get_raw_spectrum_test(i);
        shutter_test(i);
        light_source_test(i);
        data_buffer_test(i);
        speed_test(i);
        test_corrections(i);
    }

    for(i = 0; i < device_count; i++) {
        printf("\nClosing spectrometer %d.\n", i);
        flag = seabreeze_close_spectrometer(i, &error);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
    }

    seabreeze_shutdown();

    return 0;
}


void tec_test(int index) {

    int error;
    double temp;

    printf("\n\nSetting TEC enable to false\n");
    seabreeze_set_tec_enable(index, &error, 0);
    printf("...Result is [%s]\n", get_error_string(error));

    if(0 != error) {
        printf("\tBailing out of lengthy TEC test since not supported.\n");
        return;
    }

    usleep(1000000);

    printf("\nSetting TEC temperature to -5C\n");
    seabreeze_set_tec_temperature(index, &error, -5);
    printf("...Result is [%s]\n", get_error_string(error));

    printf("\nSetting TEC enable to true\n");
    seabreeze_set_tec_enable(index, &error, 1);
    printf("...Result is [%s]\n", get_error_string(error));

    printf("\nGetting TEC temperature\n");
    temp = seabreeze_read_tec_temperature(index, &error);
    printf("...Result is %1.2f C [%s]\n", temp, get_error_string(error));

    usleep(1000000);
    printf("\nGetting TEC temperature\n");
    temp = seabreeze_read_tec_temperature(index, &error);
    printf("...Result is %1.2f C [%s]\n", temp, get_error_string(error));

}

void strobe_enable_test(int index) {

    int error;

    printf("\n\nSetting strobe/lamp enable to true\n");
    seabreeze_set_strobe_enable(index, &error, 1);
    printf("...Result is [%s]\n", get_error_string(error));
    usleep(500000);
    printf("\nSetting strobe/lamp enable to false\n");
    seabreeze_set_strobe_enable(index, &error, 0);
    printf("...Result is [%s]\n", get_error_string(error));

}


void trigger_mode_test(int index) {

    int error;

    printf("\n\nSetting trigger mode to 1\n");
    seabreeze_set_trigger_mode(index, &error, 1);
    printf("...Result is [%s]\n", get_error_string(error));
    usleep(500000);
    printf("\nSetting trigger mode to 0\n");
    seabreeze_set_trigger_mode(index, &error, 0);
    printf("...Result is [%s]\n", get_error_string(error));

}

void read_serial_number_test(int index) {
    char serial_number[32];
    int flag;
    int error;

    printf("\n\nGetting serial number.\n");
    flag = seabreeze_get_serial_number(index, &error, serial_number, 32);
    printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
    serial_number[31] = '\0';
    if(flag > 0) {
        printf("\tSerial number: [%s]\n", serial_number);
    }
}

void read_eeprom_test(int index) {
    unsigned char serial_number[17];
    int flag;
    int error;

    printf("\n\nGetting EEPROM slot 1.\n");
    flag = seabreeze_read_eeprom_slot(index, &error, 1, serial_number, 17);
    printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
    serial_number[16] = '\0';
    if(flag > 0) {
        printf("\tFrom slot 1: %s\n", serial_number);
    }
}

void test_irrad_cal(int index) {
    float calibration[4096];
    int flag;
    int error;
    float area;

    printf("\n\nGetting irradiance calibration.\n");
    flag = seabreeze_read_irrad_calibration(index, &error, calibration, 4096);
    printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
    if(flag > 100) {
        printf("\tCal factor 100: %1.4e\n", calibration[100]);
    }

/* This is optional because it may alter the device.  This can be
 * enabled for testing, but be aware that any problems may erase the
 * irradiance calibration stored on the device.
 */
//#define __TEST_WRITE_IRRAD_CALIBRATION
#ifdef __TEST_WRITE_IRRAD_CALIBRATION
    if(0 != flag && 0 == error) {
        printf("\nWriting back irradiance calibration.\n");
        /* Attempt to write back the calibration */
        flag = seabreeze_write_irrad_calibration(index, &error, calibration, flag);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("\nReading back calibration:\n");
        flag = seabreeze_read_irrad_calibration(index, &error, calibration, 4096);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        if(flag > 100) {
            printf("\tCal factor 100: %1.4e\n", calibration[100]);
        }
    }
#endif /* __TEST_WRITE_IRRAD_CALIBRATION */

    printf("\nChecking whether device has irradiance collection area stored.\n");
    flag = seabreeze_has_irrad_collection_area(index, &error);
    printf("...Result is (%d) [%s]\n", flag, get_error_string(error));

    printf("\nAttempting to read irradiance collection area.\n");
    area = seabreeze_read_irrad_collection_area(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    if(0 == error) {
        printf("\tReported collection area is [%f]\n", area);
    }

#ifdef __TEST_WRITE_IRRAD_CALIBRATION
    if(0 == error) {
        printf("\nAttempting to write back collection area.\n");
        seabreeze_write_irrad_collection_area(index, &error, area);
        printf("...Result is [%s]\n", get_error_string(error));
        printf("\nReading back collection area:\n");
        area = seabreeze_read_irrad_collection_area(index, &error);
        printf("...Result is [%s]\n", get_error_string(error));
        if(0 == error) {
            printf("\tReported collection area is [%f]\n", area);
        }
    }
#endif /* __TEST_WRITE_IRRAD_CALIBRATION */

}

void get_spectrum_test(int index) {

    int error;
    int flag;
    int spec_length;
    double *spectrum = 0;

    printf("\n\nGetting formatted spectrum length.\n");
    spec_length = seabreeze_get_formatted_spectrum_length(index, &error);
    printf("...Result is (%d) [%s]\n", spec_length, get_error_string(error));

    if(spec_length > 0) {
        spectrum = (double *)calloc((size_t)spec_length, sizeof(double));

        printf("\nGetting a formatted spectrum.\n");
        flag = seabreeze_get_formatted_spectrum(index, &error, spectrum, spec_length);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("\tPixel value 20 is %1.2f\n", spectrum[20]);

        free(spectrum);
    }
}

void shutter_test(int index) {
    int     i, error, spec_length;
    int     darkCount=0;
    double *spectrum = NULL;
    double *darkSpectrum = NULL;
    
    printf("\n\nTesting the shutter\n");
    seabreeze_set_trigger_mode(index, &error, 0); // trigger to normal
    seabreeze_set_integration_time_microsec (index, &error, (unsigned long)4000000);
    printf("\nSetting device %d integration time to 4s and trigger mode to normal.\n", index);
    seabreeze_set_integration_time_microsec(index, &error, 100000);
    printf("\nAttempting to close shutter.\n");
    seabreeze_set_shutter_open(index, &error, 0);
    printf("...Command recognized, is [%s]\n", get_error_string(error));
    printf("...Taking dark spectrum.\n");
    spec_length = seabreeze_get_formatted_spectrum_length(index, &error);
    if(spec_length > 0) {
        darkSpectrum = (double *)calloc((size_t)spec_length, sizeof(double));
        seabreeze_get_formatted_spectrum(index, &error, darkSpectrum, spec_length);
    }
    
    printf("\nAttempting to open shutter.\n");
    seabreeze_set_shutter_open(index, &error, 1);
    printf("...Command recognized, is [%s]\n", get_error_string(error));
    printf("...Taking sample spectrum.\n");
    if(spec_length > 0) {
        spectrum = (double *)calloc((size_t)spec_length, sizeof(double));
        seabreeze_get_formatted_spectrum(index, &error, spectrum, spec_length);
    }
    
   
    
    // to actually test the shutter the spectra should be compared. The optical port
    //  should obviously be exposed to light. This could be done by comparing dark pixels
    //  but for now a comparison of the dark and light spectra can be done.
    for(i=0; i<spec_length; i++) {
        // are there too few spectrum pixels with values larger than the dark
        //  spectrum? If so, either there is no light, the source is a laser,
        //  the integration time is too low, the shutter isn't working or 
        //  there is no shutter.  

        if(spectrum[i]>darkSpectrum[i]*1.02){
            darkCount++;
        }
    }
    if((float)darkCount/spec_length>0.01)
        printf("\tThe shutter appears to be working.\n");
    else
        printf("\tThe shutter does not appear to be working. Confirm that there is "
            "enough light in the optical port that a spectrum would register.\n");
    free(darkSpectrum);
    free(spectrum);
}

void light_source_test(int index) {
    int error = 0;
    int light_source_count;
    int i;

    printf("\n\nAttempting to get number of light sources.\n");
    light_source_count = seabreeze_get_light_source_count(index, &error);
    printf("...Result is %d [%s]\n", light_source_count, get_error_string(error));

    for(i = 0; i < light_source_count; i++) {
        printf("\nAttempting to enable light source %d\n", i);
        seabreeze_set_light_source_enable(index, &error, i, 1);
        printf("...Result is [%s]\n", get_error_string(error));
        printf("\nAttempting to set light source %d intensity to 50%%\n", i);
        seabreeze_set_light_source_intensity(index, &error, i, 0.5);
        printf("...Result is [%s]\n", get_error_string(error));
        printf("\nAttempting to disable light source %d\n", i);
        seabreeze_set_light_source_enable(index, &error, i, 0);
        printf("...Result is [%s]\n", get_error_string(error));
    }
}

void data_buffer_test(int index) {
    int error = 0;
    unsigned long capacity = 0;
    unsigned long oldCapacity = 0;
    unsigned long maxCapacity = 0;
    unsigned long minCapacity = 0;
    unsigned long targetCapacity = 0;
    unsigned long count = 0;
    unsigned long newCount = 0;

    printf("\n\nTesting data buffering capabilities.\n");

    printf("\nAttempting to get data buffer minimum capacity\n");
    minCapacity = seabreeze_get_buffer_capacity_minimum(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    if(0 != error) {
        printf("Device does not appear to support data buffer feature.\n");
        return;
    }
    printf("Minimum capacity: %ld\n", minCapacity);
    
    printf("\nAttempting to get data buffer maximum capacity\n");
    maxCapacity = seabreeze_get_buffer_capacity_maximum(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    printf("Maximum capacity: %ld\n", maxCapacity);
    
    printf("\nAttempting to get current data buffer capacity\n");
    oldCapacity = seabreeze_get_buffer_capacity(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    printf("Current capacity: %ld\n", oldCapacity);

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
        printf("\nAttempting to set data buffer capacity to %ld\n", targetCapacity);
        seabreeze_set_buffer_capacity(index, &error, targetCapacity);
        printf("...Result is [%s]\n", get_error_string(error));

        printf("\nAttempting to get current data buffer capacity\n");
        capacity = seabreeze_get_buffer_capacity(index, &error);
        printf("...Result is [%s]\n", get_error_string(error));
        printf("Current capacity: %ld\n", capacity);

        if(capacity != targetCapacity) {
            printf("ERROR: did not get back the expected value.\n");
        }
    }
    
    printf("\nAttempting to get the number of items now in the buffer:\n");
    count = seabreeze_get_buffer_element_count(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    printf("Number of items: %ld\n", count);

    printf("\nWaiting for more data to be buffered...\n");
    sleep(2);
    
    printf("\nAttempting to get the number of items now in the buffer:\n");
    count = seabreeze_get_buffer_element_count(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    printf("Number of items: %ld\n", count);

    printf("\nAttempting to clear the buffer:\n");
    seabreeze_clear_buffer(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    
    printf("\nAttempting to get the number of items now in the buffer:\n");
    newCount = seabreeze_get_buffer_element_count(index, &error);
    printf("...Result is [%s]\n", get_error_string(error));
    printf("Number of items: %ld\n", newCount);

    if(newCount >= count && newCount > 1) {
            /* It is expected that the count would drop to zero, but there is a chance that
             * another scan could be stored between the clear and the query.  This should
             * allow at most one scan in that interval without throwing the error.
             */
            printf("ERROR: count did not seem to drop when the buffer was cleared.\n");
    }

    if(targetCapacity != oldCapacity) {
        printf("\nAttempting to set data buffer capacity back to %ld\n", oldCapacity);
        seabreeze_set_buffer_capacity(index, &error, oldCapacity);
        printf("...Result is [%s]\n", get_error_string(error));
    
        printf("\nAttempting to get current data buffer capacity\n");
        capacity = seabreeze_get_buffer_capacity(index, &error);
        printf("...Result is [%s]\n", get_error_string(error));
        printf("Current capacity: %ld\n", capacity);

        if(capacity != oldCapacity) {
            printf("ERROR: did not get back the expected value.\n");
        }
    }
}

void get_raw_spectrum_test(int index) {

    int flag;
    int error;
    int raw_length;
    unsigned char *raw_spectrum;

    printf("\n\nGetting unformatted spectrum length.\n");
    raw_length = seabreeze_get_unformatted_spectrum_length(index, &error);
    printf("...Result is (%d) [%s]\n", raw_length, get_error_string(error));

    if(raw_length > 0) {
        raw_spectrum = (unsigned char *)calloc((size_t)raw_length, sizeof(unsigned char));

        printf("\nGetting an unformatted spectrum.\n");
        flag = seabreeze_get_unformatted_spectrum(index, &error, raw_spectrum, raw_length);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("\tBuffer values 19 and 20 are 0x%02X%02X\n", raw_spectrum[19], raw_spectrum[20]);

        free(raw_spectrum);
    }
}

void get_wavelengths_test(int index) {

    int error;
    int flag;
    int spec_length;
    double *wls = 0;

    printf("\n\nGetting formatted spectrum length.\n");
    spec_length = seabreeze_get_formatted_spectrum_length(index, &error);
    printf("...Result is (%d) [%s]\n", spec_length, get_error_string(error));

    if(spec_length > 0) {
        wls = (double *)calloc((size_t)spec_length, sizeof(double));

        printf("\nGetting wavelengths.\n");
        flag = seabreeze_get_wavelengths(index, &error, wls, spec_length);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("\tPixel 20 is wavelength %1.2f nm\n", wls[20]);

        free(wls);
    }
}

void test_corrections(int index) {
    int error = 0;
    int flag;
    int spec_length;
    double *spectrum = 0;

    printf("\n\nTesting electric dark and nonlinearity correction.\n");
    if(0 == is_electric_dark_supported(index, &error)) {
        printf("...Electric dark correction is not supported for this device.\n");
        return;
    }

    printf("\nGetting formatted spectrum length.\n");
    spec_length = seabreeze_get_formatted_spectrum_length(index, &error);
    printf("...Result is (%d) [%s]\n", spec_length, get_error_string(error));

    if(spec_length > 0) {
        spectrum = (double *)calloc((size_t)spec_length, sizeof(double));

        printf("\nGetting a spectrum corrected for electrical dark.\n");
        flag = get_edark_corrected_spectrum(index, &error, spectrum, spec_length);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("\tPixel value 20 is %1.2f\n", spectrum[20]);

        printf("\nApplying nonlinearity correction\n");
        flag = do_nonlinearity_correction(index, &error, spectrum, spec_length);
        printf("...Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("\tPixel value 20 is %1.2f\n", spectrum[20]);

        free(spectrum);
    }
}

void speed_test(int index) {
    struct timeval start;
    struct timeval end;
    int error;
    unsigned char c;
    int scans = 100;
    long usec;
    int i;
    int flag;
    long minimum_time;

    printf("\n\nGetting minimum integration time from device.\n");
    minimum_time = seabreeze_get_min_integration_time_microsec(index, &error);
    printf("...Minimum is %ld microseconds, result is [%s]\n", minimum_time, get_error_string(error));
    if(minimum_time < 0) {
        /* If there was an error, reset to a time that is supported widely. */
        minimum_time = 15000;
    }

    printf("\nSetting integration time to %ld usec.\n", minimum_time);
    seabreeze_set_integration_time_microsec(index, &error, minimum_time);
    printf("...Result is [%s]\n", get_error_string(error));

    printf("Starting speed test with %d scans.\n", scans);

    gettimeofday(&start, NULL);
    for(i = 0; i < scans; i++) {
        flag = seabreeze_get_unformatted_spectrum(index, &error, &c, 1);
        if(error != 0) {
            printf("\tRead error (%s) on speed test at iteration %d\n", get_error_string(error), i);
            break;
        }
    }
    gettimeofday(&end, NULL);

    time_t secDiff = end.tv_sec - start.tv_sec;
    usec = (long)(end.tv_usec - start.tv_usec);  /* This may be negative */
    usec += secDiff * 1e6;

    printf("...Result is (%d) [%s]\n", flag, get_error_string(error));

    printf("\tAverage acquisition time (msec) over %d scans: %1.3f\n", scans, usec/(1.0 * scans * 1000));
    printf("\tTotal time elapsed: %1.2f seconds\n", usec/1e6);
}
