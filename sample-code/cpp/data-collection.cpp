/***********************************************************************//**
 * @file    data-collection.cpp
 * @date    Feb 3, 2014
 * @author  Ocean Optics, Inc
 *
 * This is a command-line utility to perform bulk data-collection via SeaBreeze.
 *
 * Invocation and arguments: see usage()
 *
 * To-Do:
 *
 * - more complex "step" syntax, including support for EDC, NLC, etc:
 *
 *     --step 'cnt=10,integ=100ms,avg=3,delay=50ms,edc=on,nlc=on,boxcar=5'
 *
 * - or, move to an external file specifying steps (possibly read at STDIN):
 *
 *   <steps>
 *     <step label="laser" count="1"  integ="10ms"  averaging="1" delay="0ms"  edc="on" nlc="on" boxcar="0" />
 *     <step label="raman" count="10" integ="100ms" averaging="3" delay="50ms" edc="on" nlc="on" boxcar="5" />
 *   </steps>
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

#include "api/SeaBreezeWrapper.h"
#include "api/seabreezeapi/SeaBreezeAPIConstants.h"
#include "common/Log.h"
#include "util.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#endif

using std::string;
using std::vector;

#define MAX_EEPROM_LENGTH   15
#define MAX_DEVICES         32
#define MAX_PIXELS        2048

////////////////////////////////////////////////////////////////////////////////
// inner types
////////////////////////////////////////////////////////////////////////////////

struct Step
{
    unsigned long scanCount;
    unsigned long integrationTimeMicrosec;
    unsigned int  scansToAverage;
    unsigned long postScanSleepMicroseconds;
};

////////////////////////////////////////////////////////////////////////////////
// constants
////////////////////////////////////////////////////////////////////////////////

static const char *rcs_id __attribute__ ((unused)) =
    "$Header: http://gforge.oceanoptics.com/svn/seabreeze/releases/Release_2014_10_01-1730-3.0/sample-code/cpp/data-collection.cpp 1215 2014-08-07 21:52:36Z mzieg $";

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////

int specIndex = 0;
int error = ERROR_SUCCESS;

unsigned iterations = 1;
string basefile = "data";
vector<Step> steps;

static const struct option opts[] =
{
    { "basefile",   required_argument, NULL, 0 },
    { "index",      required_argument, NULL, 0 },
    { "iterations", required_argument, NULL, 0 },
    { "log-level",  required_argument, NULL, 0 },
    { "step",       required_argument, NULL, 0 },
    { NULL,         no_argument,       NULL, 0 }
};

void usage() {
    puts("data-collection (C) 2014, Ocean Optics Inc\n"
         "\n"
         "Usage:\n"
         "    $ ./data-collection [--index specIndex] [--step cnt:integ:avg]+ [--iterations iter]\n"
         "                        [--basefile foo] [--log-level foo]\n"
         "\n"
         "Where:\n"
         "\n"
         "--index      takes the integral index of an enumerated Ocean Optics spectrometer (0-127)\n"
         "             (default: 0, the first or only connected spectrometer found)\n"
         "--step       zero or more colon-delimited tuples (e.g. '10:100:3'), where the first value\n"
         "             is number of acquisitions to take, the second value is the integration time in\n"
         "             microseconds, and the third value is the number of scans to average (no default);\n"
         "             if a 4th tuple is present, it will represent a post-scan sleep in microseconds\n"
         "             (scanCount may be zero for sleep-only steps)\n"
         "--iterations how many times to run the complete sequence of steps (default: 1) (0 to run forever)\n"
         "--basefile   prefix of output filename (i.e. '/path/to/data').  Will be automatically be\n"
         "             suffixed with '-iter-00000-step-00-acq-00000.csv' for each recorded acquisition.\n"
         "--log-level  one of TRACE, DEBUG, INFO, WARN, ERROR\n"
    );

    exit(1);
}

bool hasError(const char *label)
{
    if (error != ERROR_SUCCESS)
    {
        char msg[80];
        seabreeze_get_error_string(error, msg, sizeof(msg));
        seabreeze_log_error("error calling %s: %s\n", label, msg);
    }
    return error != 0;
}

void parseArgs(int argc, char **argv)
{
    int longIndex = 0;

    // iterate over cmd-line arguments
    while (1)
    {
        // read next option
        int opt = getopt_long(argc, argv, "", opts, &longIndex);

        // no more options
        if (opt == -1)
            break;

        // was this a valid argument?
        if (opt == 0)
        {
            string option(opts[longIndex].name);

            if (option == "index")
                specIndex = atoi(optarg);
            else if (option == "iterations")
                iterations = atoi(optarg);
            else if (option == "basefile")
                basefile = optarg;
            else if (option == "log-level")
                seabreeze_log_set_level_string(optarg);
            else if (option == "step")
            {
                vector<string> tuples = OOI::Util::split(string(optarg), ':');

                Step step;
                step.scanCount                 = strtol(tuples[0].c_str(), NULL, 10);
                step.integrationTimeMicrosec   = strtol(tuples[1].c_str(), NULL, 10);
                step.scansToAverage            = strtol(tuples[2].c_str(), NULL, 10);
                step.postScanSleepMicroseconds = (tuples.size() > 3)
                                               ? strtol(tuples[3].c_str(), NULL, 10)
                                               : 0;
                steps.push_back(step);
            }
            else
                usage();
        }
        else
            usage();
    }
}

int main(int argc, char **argv)
{
    seabreeze_log_set_level_string("info");

    ////////////////////////////////////////////////////////////////////////////
    // parse & validate arguments
    ////////////////////////////////////////////////////////////////////////////

    seabreeze_log_debug("processing cmd-line args");
    parseArgs(argc, argv);

    if (steps.size() == 0)
    {
        printf("ERROR: no steps provided\n");
        usage();
    }

    if (iterations > 0)
        seabreeze_log_info("The following step%s will be executed %lu time%s:",
            steps.size() > 1 ? "s" : "", iterations, iterations > 1 ? "s" : "");
    else
        seabreeze_log_info("The following step%s will be executed indefinitely:",
            steps.size() > 1 ? "s" : "");

    for (unsigned i = 0; i < steps.size(); i++)
    {
        seabreeze_log_info("Step #%u", i + 1);
        seabreeze_log_info("    scan count       = %lu", steps[i].scanCount);
        seabreeze_log_info("    integration time = %lu microsec", steps[i].integrationTimeMicrosec);
        seabreeze_log_info("    scans to average = %lu", steps[i].scansToAverage);
        seabreeze_log_info("    post-scan sleep  = %lu microsec", steps[i].postScanSleepMicroseconds);
        seabreeze_log_info("");
    }

    ////////////////////////////////////////////////////////////////////////////
    // initialize SeaBreeze
    ////////////////////////////////////////////////////////////////////////////

    // give SeaBreeze time to fully instantiate
    seabreeze_log_debug("initializing SeaBreeze");
    sleep(1);

    seabreeze_open_spectrometer(specIndex, &error);
    if (hasError("open_spectrometer"))
        exit(1);

    unsigned pixels = seabreeze_get_formatted_spectrum_length(specIndex, &error);
    if (hasError("get_formatted_spectrum_length"))
        exit(1);

    double wavelengths[pixels];
    seabreeze_get_wavelengths(specIndex, &error, wavelengths, sizeof(wavelengths));
    if (hasError("get_wavelengths"))
        exit(1);

    ////////////////////////////////////////////////////////////////////////////
    // perform data collection
    ////////////////////////////////////////////////////////////////////////////

    // iterate over iterations
    unsigned iteration = 0;
    bool done = false;

    while (!done)
    {
        seabreeze_log_debug("processing iteration #%u", iteration + 1);

        // iterate over steps
        for (unsigned stepCount = 0; stepCount < steps.size(); stepCount++)
        {
            seabreeze_log_debug("processing step #%d", stepCount + 1);
            Step& step = steps[stepCount];

            // handle sleep-only steps
            if (step.scanCount == 0)
            {
                seabreeze_log_info("sleeping %lu microseconds", step.postScanSleepMicroseconds);
                usleep(step.postScanSleepMicroseconds);
                continue;
            }

            // apply settings for this step
            seabreeze_set_integration_time_microsec(specIndex, &error, step.integrationTimeMicrosec);
            if (hasError("set_integration_time_microsec"))
                exit(1);

            // iterate over scans
            for (unsigned scanCount = 0; scanCount < step.scanCount; scanCount++)
            {
                // process acquisition
                seabreeze_log_debug("collecting scan %d of %lu", scanCount + 1, step.scanCount);
                double spectrum[pixels];
                seabreeze_get_formatted_spectrum(specIndex, &error, spectrum, sizeof(spectrum));
                if (hasError("get_formatted_spectrum"))
                    exit(1);

                // perform multi-scan averaging (used to be in SeaBreeze, now application code)
                if (step.scansToAverage > 1)
                {
                    double tmp[pixels];
                    for (unsigned acqCount = 1; acqCount < step.scansToAverage; acqCount++)
                    {
                        seabreeze_get_formatted_spectrum(specIndex, &error, tmp, sizeof(tmp));
                        if (hasError("get_formatted_spectrum"))
                            exit(1);
                        for (unsigned pixel = 0; pixel < pixels; pixel++)
                            spectrum[pixel] += tmp[pixel];
                    }
                    for (unsigned pixel = 0; pixel < pixels; pixel++)
                        spectrum[pixel] /= step.scansToAverage;
                }

                // save averaged acquisition to file
                char filename[256];
                snprintf(filename, sizeof(filename), "%s-iter-%05u-step-%02u-acq-%05u.csv",
                    basefile.c_str(), iteration + 1, stepCount + 1, scanCount + 1);
                seabreeze_log_info("saving %s", filename);
                FILE *f = fopen(filename, "w");
                if (f != NULL)
                {
                    for (unsigned pixel = 0; pixel < pixels; pixel++)
                        fprintf(f, "%.2lf,%.2lf\n", wavelengths[pixel], spectrum[pixel]);
                    fclose(f);
                }
                else
                {
                    printf("ERROR: can't write %s\n", filename);
                    exit(1);
                }

                // perform post-scan delay
                if (step.postScanSleepMicroseconds > 0)
                {
                    seabreeze_log_debug("sleeping %lu microseconds", step.postScanSleepMicroseconds);
                    usleep(step.postScanSleepMicroseconds);
                }
            }
        }

        iteration++;
        if (iterations > 0 && iteration >= iterations)
            done = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    // shutdown
    ////////////////////////////////////////////////////////////////////////////

    seabreeze_close_spectrometer(specIndex, &error);

    return 0;
}
