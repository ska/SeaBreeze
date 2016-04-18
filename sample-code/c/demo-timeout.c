/**************************************************************************//**
 * @file    demo-timeout.c
 * @date    13-Dec-2011
 * @author  Ocean Optics, Inc.
 * @brief   Demonstrate how to put a timeout on spectrometer calls,
 *          for instance to reset a device blocking on an external
 *          trigger.
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
#define MAX_DEVICES        16 //!< max number of connected spectrometers
#define TIMEOUT_MILLISEC 2500 //!< timeout spectral acquisition after 2.5sec

typedef struct THREAD_DATA {
    pthread_t       thread_id;
    pthread_cond_t  cond;
    pthread_mutex_t mut;
    int             index;
    int*            error;
    double*         buffer;
    int             length;
    int             complete;
} thread_data_t;

void *acquisition_thread_entry(void *data) {
    thread_data_t* thread_data = (thread_data_t*) data;

    pthread_mutex_lock  (&(thread_data->mut));
    pthread_mutex_unlock(&(thread_data->mut));

    seabreeze_get_formatted_spectrum(thread_data->index, thread_data->error,
        thread_data->buffer, thread_data->length);

    thread_data->complete = 1;

    pthread_cond_broadcast(&(thread_data->cond));

    return NULL;
}

/**
* @brief wraps seabreeze_get_formatted_spectrum with a timeout
* @param index:  see seabreeze_get_formatted_spectrum
* @param error:  see seabreeze_get_formatted_spectrum
* @param buffer: see seabreeze_get_formatted_spectrum
* @param length: see seabreeze_get_formatted_spectrum
* @param timeout_millis (Input) timeout in milliseconds
* @return 0 on success, ETIMEDOUT on timeout, for other
*         non-zero error codes see pthread_cond_timedwait
*/
int seabreeze_get_formatted_spectrum_timeout(int index, int *error, double *buffer,
        int length, unsigned long timeout_millis)
{
    // compute timeout as absolute deadline
    struct timespec deadline;
    struct timeval now;
    gettimeofday(&now, NULL);
    deadline.tv_sec  = now.tv_sec;
    deadline.tv_nsec = now.tv_usec * 1000 + timeout_millis * 1000 * 1000;
    while (deadline.tv_nsec >= (unsigned long) 1000 * 1000 * 1000) {
           deadline.tv_nsec -= (unsigned long) 1000 * 1000 * 1000;
           deadline.tv_sec++;
    }

    // populate thread data
    thread_data_t thread_data;
    thread_data.index     = index;
    thread_data.error     = error;
    thread_data.buffer    = buffer;
    thread_data.length    = length;
    thread_data.complete  = 0;
    thread_data.thread_id = 0;

    // lock mutex BEFORE creating thread
    pthread_mutex_init(&(thread_data.mut ), NULL);
    pthread_cond_init (&(thread_data.cond), NULL);
    pthread_mutex_lock(&(thread_data.mut));

    // create thread (won't run until mutex is released)
    pthread_create( &(thread_data.thread_id),   // will hold generated thread_id
                    NULL,                       // thread attributes
                    acquisition_thread_entry,   // thread entry point
                    (void*) &thread_data);      // handle to private thread data

    // simultaneously start thread and begin countdown
    int pthread_error = 0;
    while(!thread_data.complete && !pthread_error)
        pthread_error = pthread_cond_timedwait(&(thread_data.cond),
                                               &(thread_data.mut),
                                               &deadline);
    // cleanup
    if (pthread_error) {
        if (ETIMEDOUT != pthread_error)
            logger("%s: Unexpected error from pthread_cond_timedwait: %d",
                __FUNCTION__, pthread_error);

        pthread_cancel(thread_data.thread_id);
    }

    pthread_cond_destroy(&(thread_data.cond));
    pthread_mutex_destroy(&(thread_data.mut));

    return pthread_error;
}

void initialize_spectrometer(int index, unsigned long integ_time_millis, int trigger_mode) {
    logger("[%02d]   >> initializing spectrometer", index);
    int error = 0;

    logger("[%02d]   >>   resetting trigger mode (%d)", index, trigger_mode);
    seabreeze_set_trigger_mode(index, &error, trigger_mode);
    check_error(index, &error, "seabreeze_set_trigger_mode");

    logger("[%02d]   >>   closing", index);
    seabreeze_close_spectrometer(index, &error);
    check_error(index, &error, "seabreeze_close_spectrometer");

    logger("[%02d]   >>   re-opening", index);
    seabreeze_open_spectrometer(index, &error);
    check_error(index, &error, "seabreeze_open_spectrometer");

    logger("[%02d]   >>   setting integration time (%lums)", index, integ_time_millis);
    seabreeze_set_integration_time_microsec(index, &error, integ_time_millis * 1000);
    check_error(index, &error, "seabreeze_set_integration_time_microsec");

    logger("[%02d]   >>   initializing trigger mode (%d)", index, trigger_mode);
    seabreeze_set_trigger_mode(index, &error, trigger_mode);
    check_error(index, &error, "seabreeze_set_trigger_mode");
}

int main(int argc, char **argv) {
    int error = 0;

    logger_header("Output from %s", argv[0]);

    logger("Finding connected spectrometers...");
    int device_count = 0;
    for(int i = 0; i < SEABREEZE_MAX_DEVICES; i++)
        if(!seabreeze_open_spectrometer(i, &error))
            device_count++;
        else
            break;
    logger("%d devices found.", device_count);

    // test each connected device
    for (int i = 0; i < device_count; i++) {

        ////////////////////////////////////////////////////////////////////////
        // initialization: spectrometer type, integration time, spectrum length,
        // trigger modes, and pixel buffer
        ////////////////////////////////////////////////////////////////////////

        // spectrometer type
        char type[MAX_LABEL_SIZE + 1];
        seabreeze_get_model(i, &error, type, sizeof(type));
        if (check_error(i, &error, "seabreeze_get_model"))
            continue;

        logger_header("[%02d] Spectrometer type: %s", i, type);

        if (!strcmp(type, "STS")) {
            logger("WARNING: timeout does not currently work on the STS "
                   "(see \"Simulate trigger pulse,\" STS Data Sheet p19)");
            continue;
        }

        // get trigger modes
        int external_trigger_mode = get_external_trigger_mode(type);
        if (external_trigger_mode < 0) {
            logger("ERROR: don't know how to initialize '%s' to external triggering mode", type);
            continue;
        }

        // spectrum length
        logger("[%02d] getting formatted spectrum length", i);
        int pixels = seabreeze_get_formatted_spectrum_length(i, &error);
        if (check_error(i, &error, "seabreeze_get_formatted_spectrum_length"))
            continue;
        logger("[%02d]   formatted spectrum length = %d doubles", i, pixels);

        // apply settings
        const int default_trigger_mode = 0;
        const unsigned long integ_time_millis = 1000;
        initialize_spectrometer(i, integ_time_millis, default_trigger_mode);

        // spectrum buffer
        double *buffer = (double*) malloc(pixels * sizeof(double));

        ////////////////////////////////////////////////////////////////////////
        // Loop through several acquisitions.  On every 3rd acquisition,
        // deliberately throw the spectrometer into a blocking trigger mode,
        // to verify that the timeout monitor correctly detects and resolves
        // the deadlock
        ////////////////////////////////////////////////////////////////////////

        int success_count = 0;
        int test_attempts = 10;
        for (int j = 0; j < test_attempts; j++) {
            logger("");
            logger("[%02d] test pass %d", i, j + 1);

            // on every 3rd test, deliberately freeze the device
            if (2 == j % 3) {
                logger("[%02d] >>deliberately attempting to block device!", i);
                logger("[%02d]   setting external trigger mode (%d)", i,
                    external_trigger_mode);
                seabreeze_set_trigger_mode(i, &error, external_trigger_mode);
                if (check_error(i, &error, "seabreeze_set_trigger_mode"))
                    break;
            }

            // attempt to acquire a spectrum
            logger("[%02d]   getting formatted spectrum with timeout %d milliseconds",
                i, TIMEOUT_MILLISEC);
            int timeout_error = seabreeze_get_formatted_spectrum_timeout(i, &error, buffer,
                    pixels, TIMEOUT_MILLISEC);

            if (!timeout_error) {

                logger("[%02d]   sucessfully acquired spectrum", i);
                success_count++;

            } else if (ETIMEDOUT == timeout_error) {

                // handle timeout condition
                logger("[%02d]   TIMEOUT EXCEEDED! (resetting trigger mode %d)", i,
                    default_trigger_mode);

                // perform a minimal reset of the device
                seabreeze_set_trigger_mode(i, &error, default_trigger_mode);
                if (check_error(i, &error, "seabreeze_set_trigger_mode"))
                    break;

                // ...or, you could opt for a heavier reset:
                // initialize_spectrometer(i, integ_time_millis, default_trigger_mode);

            } else {
                logger("[%02d]   ERROR: unexpected code from "
                    "seabreeze_get_formatted_spectrum_timeout (%d)",
                    i, timeout_error);
            }
        }

        // report results
        logger("");
        logger("[%02d] successfully acquired %d of %d spectra (%d timeouts)",
            i, success_count, test_attempts, test_attempts - success_count);

        // cleanup this device
        free((void*) buffer);
        buffer = NULL;
    }

    // close connected spectrometers
    logger("");
    for (int i = 0; i < device_count; i++) {
        logger("[%02d] closing", i);
        error = 0;
        seabreeze_close_spectrometer(i, &error);
        check_error(i, &error, "seabreeze_close_spectrometer");
    }
}
