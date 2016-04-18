/**************************************************************************//**
 * @file    demo-pthreads.c
 * @date    24-Apr-2014
 * @author  Ocean Optics, Inc.
 * @brief   Demonstrate how to efficiently command multiple spectrometers in
 *          parallel.
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
#include <unistd.h>
#include <signal.h>
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

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

#define MAX_LABEL_SIZE 16   // includes NULL

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

typedef struct DEVICE_DATA
{
    pthread_t       thread_id;
    pthread_mutex_t mut;

    int             index;
    int             error;
    int             pixels;
    int             running;
    unsigned long   integration_time_microsec;
    unsigned long   min_integration_time_microsec;
    double*         spectrum;
    unsigned        scan_count;
    unsigned        last_scan_count;

    char            serial[MAX_LABEL_SIZE];
    char            model[MAX_LABEL_SIZE];
} device_data_t;

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

// connected spectrometers
unsigned device_count = 0;
device_data_t* devices = NULL;

// synchronize shutdown
int shutdown_pending = 0;                   // when monitor thread should quit
unsigned long stop_time = 0;
pthread_mutex_t shutdown_mutex;

// monitor timer
struct sigaction monitor_sa;
struct itimerval monitor_timer;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

// Each spectrometer will run this function in a different thread.
// Each parallel invocation of this function will have a pointer
// to one spectrometer's device_data_t record.  Returning from
// this function will terminate the associated thread.
void *acquisition_thread_entry(void *data)
{
    // get a handle to this thread's spectrometer index,
    // spectrum buffer, mutex etc
    device_data_t* device = (device_data_t*) data;
    logger("Entering %s %s acquisition thread", device->model, device->serial);

    // each spectrometer's mutex is pre-locked before the thread
    // is started, so the thread cannot advance beyond this pair
    // of lines until the creating thread "releases" all the threads
    // by unlocking the associated mutexes
    pthread_mutex_lock  (&(device->mut));
    pthread_mutex_unlock(&(device->mut));

    // now the acquisition sequence is well and truly begun
    // logger("Starting %s %s acquisition thread (will quit at %lu)", device->model, device->serial, stop_time);
    device->running = 1;
    while (time(NULL) < stop_time && !shutdown_pending)
    {
        seabreeze_get_formatted_spectrum(device->index, &(device->error), device->spectrum, device->pixels);
        device->scan_count++;
    }
    shutdown_pending = 1;
    device->running = 0;

    pthread_mutex_lock(&shutdown_mutex);
    {
        device->running = 0;
        pthread_mutex_destroy(&(device->mut));
        free(device->spectrum);
        seabreeze_close_spectrometer(device->index, &(device->error));

    }
    pthread_mutex_unlock(&shutdown_mutex);

    return NULL;
}

// the scrolling acquisition monitor runs in this thread
void monitor_timer_entry(int signum)
{
    if (shutdown_pending)
        return;

    // generate ASCII art status row for all threads
    char line[80];
    memset(line, 0, sizeof(line));
    for (int i = 0; i < device_count; i++)
    {
        unsigned scan_count = devices[i].scan_count;
        unsigned last_scan_count = devices[i].last_scan_count;

        char *msg = "?";
        if (!devices[i].running)
            msg = "X";
        else if (scan_count == last_scan_count)
            msg = "|";
        else
        {
            msg = "V (NEW SCAN)";
            devices[i].last_scan_count = scan_count;
        }
        snprintf(line, sizeof(line), "%s%-16s", line, msg);
    }
    logger(line);
}

int main(int argc, char **argv)
{
    int error = 0;

    logger_header("Output from %s", argv[0]);

    ////////////////////////////////////////////////////////////////////////////
    // Initialize Devices
    ////////////////////////////////////////////////////////////////////////////

    logger("Finding connected spectrometers...");
    for(int i = 0; i < SEABREEZE_MAX_DEVICES; i++)
        if(!seabreeze_open_spectrometer(i, &error))
            device_count++;
        else
            break;
    logger("%d devices found", device_count);
    if (!device_count)
        exit(0);

    // allocate storage for each device
    devices = (device_data_t*) malloc(device_count * sizeof(device_data_t));
    memset(devices, 0, device_count * sizeof(device_data_t));

    // initialize each device (serially, in the main thread)
    logger("Initializing devices...");
    for (int i = 0; i < device_count; i++)
    {
        device_data_t* device = &(devices[i]);

        device->index = i;

        device->pixels = seabreeze_get_formatted_spectrum_length(i, &error);
        if (check_error(i, &error, "seabreeze_get_formatted_spectrum_length"))
            exit(1);

        seabreeze_get_model(i, &error, device->model, sizeof(device->model));
        if (check_error(i, &error, "seabreeze_get_model"))
            exit(1);

        seabreeze_get_serial_number(i, &error, device->serial, sizeof(device->serial));
        if (check_error(i, &error, "seabreeze_get_serial"))
            exit(1);

        device->min_integration_time_microsec = seabreeze_get_min_integration_time_microsec(i, &error);
        if (check_error(i, &error, "seabreeze_get_min_integration_time_microsec"))
            exit(1);

        device->spectrum = (double*) malloc(device->pixels * sizeof(double));

        logger("  Index %2d: %15s %-15s (%4u pixels)",
            device->index, device->model, device->serial, device->pixels);
    }

    // initialize shutdown mutex (looks like seabreeze_close_spectrometer()
    // may not be thread-safe?)
    pthread_mutex_init(&shutdown_mutex, NULL);

    ////////////////////////////////////////////////////////////////////////////
    // Collect Test Parameters
    ////////////////////////////////////////////////////////////////////////////

    // prompt for integration time for each device
    printf("\n");
    for (int i = 0; i < device_count; i++)
    {
        device_data_t* device = &(devices[i]);
        unsigned long microsec = 0;
        do
        {
            printf("Enter integration time for %s %s (microsec, min %luus): ",
                device->model, device->serial, device->min_integration_time_microsec);
            scanf("%lu", &microsec);
        } while (microsec < device->min_integration_time_microsec);

        device->integration_time_microsec = microsec;
        seabreeze_set_integration_time_microsec(
            device->index,
            &(device->error),
            device->integration_time_microsec);
        if (check_error(i, &error, "seabreeze_set_integration_time_microsec"))
            exit(1);
        // logger("set %s %s integration time to %lu millisec", device->model, device->serial, device->integration_time_microsec / 1000);
    }

    // prompt for max runtime
    printf("\n");
    unsigned max_seconds = 0;
    printf("Enter the maximum test runtime in seconds (can always be broken with ^C): ");
    scanf("%u", &max_seconds);

    ////////////////////////////////////////////////////////////////////////////
    // Create Threads
    ////////////////////////////////////////////////////////////////////////////

    logger("Creating acquisition threads...");
    for (int i = 0; i < device_count; i++)
    {
        device_data_t* device = &(devices[i]);

        // lock each device's mutex BEFORE creating thread, so they'll spawn
        // in a blocked state
        pthread_mutex_init(&(device->mut ), NULL);
        pthread_mutex_lock(&(device->mut));

        // create each thread (they won't start until each mutex is released)
        logger("  creating thread for %s %s", device->model, device->serial);
        pthread_create( &(device->thread_id),       // will hold generated thread_id
                        NULL,                       // thread attributes
                        acquisition_thread_entry,   // thread entry point
                        (void*) device);            // handle to private thread data
    }

    // sleep one second to allow threads to all line up at their mutex (otherwise
    // logging lines will interlace with the following prompt)
    sleep(1);

    // prompt user to begin test
    {
        char response[8];
        memset(response, 0, sizeof(response));
        while (strncasecmp(response, "START", 5))
        {
            printf("\nType 'start' to begin test: ");
            fgets(response, sizeof(response), stdin);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Start Monitor Timer
    ////////////////////////////////////////////////////////////////////////////

    {
        // configure monitor signal handler
        memset(&monitor_sa, 0, sizeof(monitor_sa));
        sigemptyset(&monitor_sa.sa_mask);
        monitor_sa.sa_flags = 0;
        monitor_sa.sa_handler = &monitor_timer_entry;
        sigaction(SIGALRM, &monitor_sa, NULL);

        // configure monitor timer
        monitor_timer.it_value.tv_sec = 0;
        monitor_timer.it_value.tv_usec = 100000;
        monitor_timer.it_interval.tv_sec = 0;
        monitor_timer.it_interval.tv_usec = 100000;         // tick monitor at 100ms
        int err = setitimer(ITIMER_REAL, &monitor_timer, NULL);
        if (err != 0)
            logger("[setitimer] err = %d (errno %d)", err, errno);
    }

    // print header row
    {
        char line[80];
        memset(line, 0, sizeof(line));
        for (int i = 0; i < device_count; i++)
            snprintf(line, sizeof(line), "%s%-16s", line, devices[i].serial);
        logger(line);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Release the Frog
    ////////////////////////////////////////////////////////////////////////////

    // first, decide when to stop
    {
        unsigned long now = time(NULL);
        stop_time = now + max_seconds;
    }

    // start threads
    // logger("Unlocking threads...");
    for (int i = 0; i < device_count; i++)
    {
        device_data_t* device = &(devices[i]);
        // logger("  unlocking %s %s", device->model, device->serial);
        pthread_mutex_unlock(&(device->mut));
    }

    // ...test executes here, inside threads

    // wait for threads to complete
    for (int i = 0; i < device_count; i++)
    {
        device_data_t* device = &(devices[i]);

        // logger("Waiting on %s %s (thread %012x) to complete", device->model, device->serial, device->thread_id);
        pthread_join(device->thread_id, NULL);
        logger("Joined %s %s (thread %012x complete)", device->model, device->serial, device->thread_id);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Shutdown
    ////////////////////////////////////////////////////////////////////////////

    free(devices);
    pthread_mutex_destroy(&shutdown_mutex);

    logger("All threads destroyed.");
}
