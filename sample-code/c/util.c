/**
* @file     util.c
* @brief    utility routines used by different test/demo applications
* @author   Mark Zieg <mark.zieg@oceanoptics.com>
* @date     Jan 2011
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
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
#include "api/SeaBreezeWrapper.h"

#define GRAPH_WIDTH    80    // columns
#define GRAPH_HEIGHT   20    // rows

#ifndef MAXFLOAT
#define MAXFLOAT	   3.40282347e+38F
#endif

pthread_mutex_t logger_mut;

/**
* @brief wrapper over printf to force flush after every call
*
* This is very handy when tests die with a lot of buffered output undisplayed.
* Also adds a timestamp and thread_id for timing/concurrency tests.
*/
void logger(const char *fmt, ...)
{
    static int logger_mut_initialized = 0;
    if (!logger_mut_initialized)
    {
        pthread_mutex_init(&logger_mut, NULL);
        logger_mut_initialized = 1;
    }
    pthread_mutex_lock(&logger_mut);

    va_list args;
    va_start(args, fmt);

    // get timestamp
    struct timeval tp;
    gettimeofday(&tp, 0);
    time_t curtime = tp.tv_sec;
    struct tm *t = localtime(&curtime);

    // get thread_id
    unsigned long thread_id = (unsigned long) pthread_self();

    // print the timestamp and thread_id prefix
    printf("%02d:%02d:%02d.%03d [%012lx] ", t->tm_hour, t->tm_min, t->tm_sec, (int)(tp.tv_usec/1000), thread_id);

    // print the actual log message
    vprintf(fmt, args);
    va_end(args);
    if (fmt[strlen(fmt)-1] != '\n')
        printf("\n");
    fflush(stdout);
    fflush(stderr);

    pthread_mutex_unlock(&logger_mut);
}

// return x - y in microsec
long diff_microsec(struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec)
    {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000)
    {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.  tv_usec is certainly positive. */
    long tv_sec = x->tv_sec - y->tv_sec;
    long tv_usec = x->tv_usec - y->tv_usec;

    return tv_sec * 1000000 + tv_usec;
}

void logger_header(const char *fmt, ...)
{
    static char line[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    logger("\n");
    logger("================================================\n");
    logger(line);
    logger("================================================\n");
    logger("\n");
}

const char* get_error_string(int error)
{
    static char buffer[32];
    seabreeze_get_error_string(error, buffer, sizeof(buffer));
    return buffer;
}

// returns non-zero if error occurred
int check_error(int index, int *error, const char *func)
{
    if (*error)
        logger("[%02d]   error calling %s: %s\n", index, func,
            get_error_string(*error));
    fflush(stdout);
    return *error;
}

// you'd kind of think the SeaBreezeWrapper would provide this...
int get_external_trigger_mode(const char *type)
{
         if (!strcmp(type, "HR4000"     )) return 0x01;
    else if (!strcmp(type, "QE65000"    )) return 0x01;
    else if (!strcmp(type, "STS"        )) return 0x01;
    else if (!strcmp(type, "USB2000"    )) return 0x03;
    else if (!strcmp(type, "USB2000PLUS")) return 0x02;
    logger("ERROR: can't find 'external' triggering mode for spectrometer type '%s'\n", type);
    return -1;
}

//! display a spectrum as a simple ASCII histogram
void graph(const char *type, const char *label, double *spectrum, int pixels)
{
    double histogram[GRAPH_WIDTH];

    // initialize histogram
    for (int i = 0; i < GRAPH_WIDTH; i++)
        histogram[i] = 0;

    // aggregate wavelength sums by bin
    for (int i = 0; i < pixels; i++)
    {
        int col = (int) ((i * GRAPH_WIDTH) / pixels);
        histogram[col] += spectrum[i];
    }

    // normalize heights
    double min = histogram[0];
    double max = histogram[0];
    for (int i = 1; i < GRAPH_WIDTH; i++)
    {
        min = histogram[i] < min ? histogram[i] : min;
        max = histogram[i] > max ? histogram[i] : max;
    }

    // display graph
    printf("[%s] %s (%d histogrammed bins range from min %.2lf, max %.2lf)\n", type, label, GRAPH_WIDTH, min, max);
    for(int row = GRAPH_HEIGHT; row; row--)
    {
        printf("  | ");
        for(int col = 0; col < GRAPH_WIDTH; col++)
            printf("%c", histogram[col] - min >= (max - min) * row / GRAPH_HEIGHT ? '*' : ' ');
        printf("\n");
    }

    // print x-axis
    printf("  |_");
    for(int col = 0; col < GRAPH_WIDTH; col++)
        printf("_");
    printf("\n\n");
    fflush(stdout);
}
