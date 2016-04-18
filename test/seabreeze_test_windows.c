/*******************************************************
 * File:    seabreeze_test_windows.c
 * Date:    July 2009
 * Author:  Ocean Optics, Inc.
 *
 * This is a test program to exercise some of the
 * SeaBreeze functionality.  This will probably not compile
 * under gcc (Linux).
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
#include <time.h>
#include "api/SeaBreezeWrapper.h"
#include <windows.h>

void speed_test(int index);
void get_spectrum_test(int index);
void get_raw_spectrum_test(int index);
void get_wavelengths_test(int index);
void read_eeprom_test(int index);
void strobe_enable_test(int index);
void trigger_mode_test(int index);
void tec_test(int index);
void data_buffer_test(int index);

const char* get_error_string(int error) {
    static char buffer[32];
    seabreeze_get_error_string(error, buffer, sizeof(buffer));
    return buffer;
}

int run_all_tests() {
    int flag;
    int error;
    char type[32];
    int device_count = 0;
    int i;

    for(i = 0; i < SEABREEZE_MAX_DEVICES; i++) {
        printf("Opening spectrometer %d.\n", i);
        flag = seabreeze_open_spectrometer(i, &error);
        printf("Result is (%d) [%s]\n", flag, get_error_string(error));
        if(0 == flag) {
            device_count++;
        } else {
            break;
        }
    }

    printf("%d devices found.\n", device_count);

    for(i = 0; i < device_count; i++) {
        printf("Getting device %d name.\n", i);
        seabreeze_get_model(i, &error, type, sizeof(type));
        printf("Result is (%s) [%s]\n", type, get_error_string(error));
    }

    for(i = 0; i < device_count; i++) {
        printf("Setting device %d integration time to 100ms.\n", i);
        seabreeze_set_integration_time_microsec(i, &error, 100000);
        printf("Result is [%s]\n", get_error_string(error));
    }

    for(i = 0; i < device_count; i++) {
        printf("Beginning test suite for device %d\n", i);
        tec_test(i);
        strobe_enable_test(i);
        trigger_mode_test(i);
        read_eeprom_test(i);
        get_wavelengths_test(i);
        get_spectrum_test(i);
        get_raw_spectrum_test(i);
        data_buffer_test(i);
        speed_test(i);
    }

    for(i = 0; i < device_count; i++) {
        printf("Closing spectrometer %d.\n", i);
        flag = seabreeze_close_spectrometer(i, &error);
        printf("Result is (%d) [%s]\n", flag, get_error_string(error));
    }

    return 0;
}

void tec_test(int index) {

    int error;
    double temp;

    printf("Setting TEC enable to false\n");
    seabreeze_set_tec_enable(index, &error, 0);
    printf("Result is [%s]\n", get_error_string(error));

    if(0 != error) {
        printf("Bailing out of lengthy TEC test since not supported.\n");
        return;
    }

    Sleep(1000);

    printf("Setting TEC temperature to -5C\n");
    seabreeze_set_tec_temperature(index, &error, -5);
    printf("Result is [%s]\n", get_error_string(error));

    printf("Setting TEC enable to true\n");
    seabreeze_set_tec_enable(index, &error, 1);
    printf("Result is [%s]\n", get_error_string(error));

    printf("Getting TEC temperature\n");
    temp = seabreeze_read_tec_temperature(index, &error);
    printf("Result is %1.2f C [%s]\n", temp, get_error_string(error));

    Sleep(1000);
    printf("Getting TEC temperature\n");
    temp = seabreeze_read_tec_temperature(index, &error);
    printf("Result is %1.2f C [%s]\n", temp, get_error_string(error));

}

void strobe_enable_test(int index) {

    int error;

    printf("Setting strobe/lamp enable to true\n");
    seabreeze_set_strobe_enable(index, &error, 1);
    printf("Result is [%s]\n", get_error_string(error));
    Sleep(500);
    printf("Setting strobe/lamp enable to false\n");
    seabreeze_set_strobe_enable(index, &error, 0);
    printf("Result is [%s]\n", get_error_string(error));

}


void trigger_mode_test(int index) {

    int error;

    printf("Setting trigger mode to 3\n");
    seabreeze_set_trigger_mode(index, &error, 3);
    printf("Result is [%s]\n", get_error_string(error));
    Sleep(500);
    printf("Setting trigger mode to 0\n");
    seabreeze_set_trigger_mode(index, &error, 0);
    printf("Result is [%s]\n", get_error_string(error));

}

void read_eeprom_test(int index) {
    unsigned char serial_number[17];
    int flag;
    int error;

    printf("Getting EEPROM slot 0.\n");
    flag = seabreeze_read_eeprom_slot(index, &error, 0, serial_number, 17);
    printf("Result is (%d) [%s]\n", flag, get_error_string(error));
    serial_number[16] = '\0';
    if(flag > 0) {
        printf("From slot 0: %s\n", serial_number);
    }
}

void get_spectrum_test(int index) {

    int error;
    int flag;
    int spec_length;
    double *spectrum = 0;

    printf("Getting formatted spectrum length.\n");
    spec_length = seabreeze_get_formatted_spectrum_length(index, &error);
    printf("Result is (%d) [%s]\n", spec_length, get_error_string(error));

    if(spec_length > 0) {
        spectrum = (double *)calloc((size_t)spec_length, sizeof(double));

        printf("Getting a formatted spectrum.\n");
        flag = seabreeze_get_formatted_spectrum(index, &error, spectrum, spec_length);
        printf("Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("Pixel value 20 is %1.2f\n", spectrum[20]);

        free(spectrum);
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
    Sleep(2000);

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

    printf("Getting unformatted spectrum length.\n");
    raw_length = seabreeze_get_unformatted_spectrum_length(index, &error);
    printf("Result is (%d) [%s]\n", raw_length, get_error_string(error));

    if(raw_length > 0) {
        raw_spectrum = (unsigned char *)calloc((size_t)raw_length, sizeof(unsigned char));

        printf("Getting an unformatted spectrum.\n");
        flag = seabreeze_get_unformatted_spectrum(index, &error, raw_spectrum, raw_length);
        printf("Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("Buffer values 19 and 20 are 0x%02X%02X\n", raw_spectrum[19], raw_spectrum[20]);

        free(raw_spectrum);
    }
}

void get_wavelengths_test(int index) {

    int error;
    int flag;
    int spec_length;
    double *wls = 0;

    printf("Getting formatted spectrum length.\n");
    spec_length = seabreeze_get_formatted_spectrum_length(index, &error);
    printf("Result is (%d) [%s]\n", spec_length, get_error_string(error));

    if(spec_length > 0) {
        wls = (double *)calloc((size_t)spec_length, sizeof(double));

        printf("Getting wavelengths.\n");
        flag = seabreeze_get_wavelengths(index, &error, wls, spec_length);
        printf("Result is (%d) [%s]\n", flag, get_error_string(error));
        printf("Pixel 20 is wavelength %1.2f nm\n", wls[20]);

        free(wls);
    }
}

void speed_test(int index) {
    clock_t start;
    clock_t end;
    int error;
    unsigned char c;
    int scans = 100;
    int i;
    int flag;
    long minimum_time;
    double secDiff;

    printf("Getting minimum integration time from device.\n");
    minimum_time = seabreeze_get_min_integration_time_microsec(index, &error);
    printf("Minimum is %ld microseconds, result is [%s]\n", minimum_time, get_error_string(error));
    if(minimum_time < 0) {
        /* If there was an error, reset to a time that is supported widely. */
        minimum_time = 15000;
    }

    printf("Setting integration time to %ld usec.\n", minimum_time);
    seabreeze_set_integration_time_microsec(index, &error, minimum_time);
    printf("Result is [%s]\n", get_error_string(error));

    printf("Starting speed test with %d scans.\n", scans);

    start = clock();
    for(i = 0; i < scans; i++) {
        flag = seabreeze_get_unformatted_spectrum(index, &error, &c, 1);
        if(error != 0) {
            printf("Read error (%s) on speed test at iteration %d\n", get_error_string(error), i);
            break;
        }
    }
    end = clock();

    secDiff = (double)(end - start)/CLOCKS_PER_SEC;

    printf("Result is (%d) [%s]\n", flag, get_error_string(error));

    printf("Average acquisition time (msec) over %d scans: %1.3f\n", scans, secDiff/(1.0 * scans / 1000.0));
    printf("Total time elapsed: %1.2f seconds\n", secDiff);
}

int main() {
    int ec = run_all_tests();

    // Provided in case the user ran the application in a new terminal window,
    // which would otherwise close (destroying the output) upon exit.
    char line[80];
    do {
        printf("\nType \"quit\" to exit...");
        fflush(stdout);

        memset(line, 0, 80);
        fgets(line, sizeof(line), stdin);

    } while (strncmp(line, "quit", 4));
    printf("\ndone.\n");

    return ec;
}
