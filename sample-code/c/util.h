/**
* @file     util.h
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

#ifndef SEABREEZE_TEST_UTIL_H
#define SEABREEZE_TEST_UTIL_H

#include <sys/time.h>

const char* get_error_string(int error);
int  check_error(int index, int *error, const char *func);
int  get_external_trigger_mode(const char *type);
void graph(const char *type, const char *label, double *spectrum, int pixels);
void logger(const char *fmt, ...);
void logger_header(const char *fmt, ...);
long diff_microsec(struct timeval *x, struct timeval *y);

#endif
