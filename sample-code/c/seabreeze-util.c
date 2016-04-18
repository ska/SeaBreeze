/***************************************************//**
 * @file    seabreeze-util.c
 * @date    Dec 5, 2011
 * @author  Ocean Optics, Inc
 *
 * This is a command-line utility to access some of the
 * SeaBreeze functionality.
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
#include <stdarg.h>
#include "api/SeaBreezeWrapper.h"
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LABEL_SIZE    15  // size of EEPROM field value (doesn't include final NULL)
#define MAX_DEVICES       32  // max number of connected spectrometers
#define MAX_PIXELS      4096  // don't think any devices have higher resolution than this

#define LOG_DEBUG(s) do { if(gArgs.debug) logger s ; } while(0)

static const char *rcs_id __attribute__ ((unused)) =
    "$Header: http://gforge.oceanoptics.com/svn/seabreeze/releases/Release_2014_10_01-1730-3.0/sample-code/c/seabreeze-util.c 1215 2014-08-07 21:52:36Z mzieg $";

struct global_args_t {
    int         debug;
    int         index;
    int         list;
    int         listDescriptors;
    int         listEEPROMs;
    int         listIrrad;
    int         listEDCPixels;
    int         eepromIndex;
    int         getSpectrum;
    int         raw;
    long        integrationTimeUS;
    const char *eepromValueHex;
    const char *eepromValueAscii;
    const char *irradFilename;
    const char *serialNumber;
    const char *serialNumberNew;
    const char *type;
} gArgs;

static const char *optString = "di:ls:t:h?";
static const struct option opts[] = {
    { "debug",              no_argument,        NULL, 'd' }, // for seabreeze-util debugging
    { "eeprom-value-hex",   required_argument,  NULL,  0  }, // long-only
    { "eeprom-value-ascii", required_argument,  NULL,  0  }, // long-only
    { "get-spectrum",       no_argument,        NULL,  0  }, // long-only
    { "raw",                no_argument,        NULL,  0  }, // long-only
    { "index",              required_argument,  NULL, 'i' },
    { "integration-time-us",required_argument,  NULL,  0  }, // long-only
    { "list",               no_argument,        NULL, 'l' },
    { "list-descriptors",   no_argument,        NULL,  0  }, // long-only
    { "list-eeproms",       required_argument,  NULL, 'e' },
    { "list-irrad",         no_argument,        NULL, 'r' },
    { "list-edc-pixels",    no_argument,        NULL,  0  }, // long-only
    { "serial-number",      required_argument,  NULL, 's' },
    { "set-eeprom-index",   required_argument,  NULL,  0  }, // long-only
    { "set-irrad",          required_argument,  NULL,  0  }, // long-only
    { "set-serial-number",  required_argument,  NULL,  0  }, // long-only
    { "type",               required_argument,  NULL, 't' },
    { "help",               no_argument,        NULL, 'h' },
    { NULL,                 no_argument,        NULL,  0  }
};

int connected_device_count = 0;

void usage() {
    puts("seabreeze-util (C) 2011, Ocean Optics, Inc\n"
         "\n"
         "Usage:\n"
         "  $ seabreeze-util [--index i] [--serial-number sn] [--type t]\n"
         "                   [--get-spectrum [--integration-time-us n {default 100ms}]]\n"
         "                   [--list] [--list-irrad] [--list-eeproms n]\n"
         "                   [--list-descriptors] [--list-edc-pixels]\n"
         "                   [--set-serial-number new_sn] [--set-irrad file]\n"
         "                   [--set-eeprom-index n] [--eeprom-value-hex ff0e43ff002000000000000000000000]\n"
         "                                          [--eeprom-value-ascii 200.4620379]\n"
         "                   [--debug]\n"
         "\n"
         "Examples:\n"
         "  $ seabreeze-util --list\n"
         "  $ seabreeze-util --list-eeproms 31 --type USB4000\n"
         "  $ seabreeze-util --list-irrad\n"
         "  $ seabreeze-util --set-irrad /path/to/file_of_floats.txt\n"
         "  $ seabreeze-util --set-eeprom-index 2 --eeprom-value-ascii 200.4620379\n"
         "  $ seabreeze-util --get-spectrum --integration-time-us 7229\n"
    );
    exit(1);
}

void parseArgs(int argc, char **argv) {

    // initialize
    gArgs.debug             = 0;
    gArgs.getSpectrum       = 0;
    gArgs.raw               = 0;
    gArgs.integrationTimeUS = 100000; // default to 100ms
    gArgs.list              = 0;
    gArgs.listDescriptors   = 0;
    gArgs.listEEPROMs       = 0;
    gArgs.listEDCPixels     = 0;
    gArgs.listIrrad         = 0;
    gArgs.index             = -1;
    gArgs.eepromIndex       = 0;
    gArgs.eepromValueHex    = NULL;
    gArgs.eepromValueAscii  = NULL;
    gArgs.irradFilename     = NULL;
    gArgs.serialNumber      = NULL;
    gArgs.type              = NULL;

    int longIndex = 0;
    int opt = getopt_long(argc, argv, optString, opts, &longIndex);
    while (opt != -1) {
        switch (opt) {
            case 'd': gArgs.debug = 1;                  break;
            case 'e': gArgs.listEEPROMs = atoi(optarg); break;
            case 'i': gArgs.index = atoi(optarg);       break;
            case 'l': gArgs.list = 1;                   break;
            case 'r': gArgs.listIrrad = 1;              break;
            case 's': gArgs.serialNumber = optarg;      break;
            case 't': gArgs.type = optarg;              break;
            case  0 :
                // long options which don't have a short-option equivalent
                     if (!strcmp("set-serial-number",  opts[longIndex].name)) gArgs.serialNumberNew   = optarg; 
                else if (!strcmp("set-irrad",          opts[longIndex].name)) gArgs.irradFilename     = optarg; 
                else if (!strcmp("get-spectrum",       opts[longIndex].name)) gArgs.getSpectrum       = 1; 
                else if (!strcmp("raw",                opts[longIndex].name)) gArgs.raw               = 1; 
                else if (!strcmp("integration-time-us",opts[longIndex].name)) gArgs.integrationTimeUS = atol(optarg); 
                else if (!strcmp("list-descriptors",   opts[longIndex].name)) gArgs.listDescriptors   = 1; 
                else if (!strcmp("list-edc-pixels",    opts[longIndex].name)) gArgs.listEDCPixels     = 1; 
                else if (!strcmp("set-eeprom-index",   opts[longIndex].name)) gArgs.eepromIndex       = atoi(optarg); 
                else if (!strcmp("eeprom-value-hex",   opts[longIndex].name)) gArgs.eepromValueHex    = optarg; 
                else if (!strcmp("eeprom-value-ascii", opts[longIndex].name)) gArgs.eepromValueAscii  = optarg; 
                else 
                    usage();
                break;
            case 'h': // default...
            case '?': // default...
            default :
                usage();
                break;
        }
        opt = getopt_long(argc, argv, optString, opts, &longIndex);
    }

    // any --list-foo option implies --list
    if (  gArgs.listIrrad     ||
          gArgs.listEEPROMs   ||
          gArgs.listEDCPixels ||
          gArgs.listDescriptors )
        gArgs.list = 1;

    // default to usage if no command was found
    if (!gArgs.list            && 
        !gArgs.getSpectrum     &&
        !gArgs.serialNumberNew && 
        !gArgs.irradFilename   && 
        !gArgs.eepromValueHex  &&
        !gArgs.eepromValueAscii) {
        usage();
    }
}

void cleanUpAndClose(int exitCode) {
    int error = 0;
    for (int i = 0; i < connected_device_count; i++) {
        LOG_DEBUG(("[%02d] closing", i));
        seabreeze_close_spectrometer(i, &error);
        if (error) {
            printf("[%02d] Error closing spectrometer: %s\n", i, get_error_string(error));
        }
    }
    exit(exitCode);
}

unsigned char hexToDec(char c) {
    unsigned char retval = 0; 
    if ('0' <= c && c <= '9')
        retval = c - '0';
    else if ('a' <= c && c <= 'f')
        retval = 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        retval = 10 + c - 'A';
    return retval;
}

void renderHex(unsigned char *buf, int len, const char* s) {
    for (int i = 0; i < len; i++) {
        unsigned char msb = hexToDec(s[i * 2 + 0]);
        unsigned char lsb = hexToDec(s[i * 2 + 1]);
        buf[i] = (msb << 4) | lsb;
    }
}

int main(int argc, char **argv) {
    int error = 0;

    parseArgs(argc, argv);

    if (gArgs.debug)
        seabreeze_set_verbose(1);

    sleep(1);

    ////////////////////////////////////////////////////////////////////////////
    // count connected devices
    ////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < SEABREEZE_MAX_DEVICES; i++) {
        if (!seabreeze_open_spectrometer(i, &error)) {
            connected_device_count++;
        } else {
            break;
        }
    }

    LOG_DEBUG(("%d devices found", connected_device_count));
    if (!connected_device_count) {
        puts("ERROR: no spectrometers found");
        exit(1);
    }

    ////////////////////////////////////////////////////////////////////////////
    // look for desired spectrometer
    ////////////////////////////////////////////////////////////////////////////

    int matching_device_count = 0;
    int matching_devices[MAX_DEVICES];

    // iterate over connected devices, attempting to match on model & serial
    char thisType[MAX_LABEL_SIZE + 1];
    char thisSerialNumber[MAX_LABEL_SIZE + 1];
    memset(thisSerialNumber, 0, sizeof(thisSerialNumber));

    // header if requested
    if (gArgs.list) {
        puts("## Type            Serial Number");
        puts("-- --------------- ---------------");
    }

    for(int i = 0; i < connected_device_count; i++) {
        int potential_match = 1;

        // match on index
        if (gArgs.index >= 0) {
            if (i == gArgs.index) {
                LOG_DEBUG(("[%02d] matches desired index"));
            } else {
                LOG_DEBUG(("[%02d] does not match desired index"));
                potential_match = 0;
            }
        }

        // match on spectrometer type
        if (potential_match) {
            seabreeze_get_model(i, &error, thisType, sizeof(thisType));
            if (!error) {
                if (gArgs.type) {
                    if (!strcmp(thisType, gArgs.type)) {
                        LOG_DEBUG(("[%02d] matches desired spectrometer type '%s'", i, thisType));
                    } else {
                        LOG_DEBUG(("[%02d] does not match desired spectrometer type '%s' (%s)", i, gArgs.type, thisType));
                        potential_match = 0;
                    }
                } else {
                    LOG_DEBUG(("[%02d] has spectrometer type '%s' (no match criteria provided)", i, thisType));
                }
            } else {
                printf("[%02d] Error getting type: %s\n", i, get_error_string(error));
                potential_match = 0;
            }
        }

        // match on serial_number
        if (potential_match) {
            seabreeze_get_serial_number(i, &error, thisSerialNumber, sizeof(thisSerialNumber));
            if (!error) {
                if (gArgs.serialNumber) {
                    if (!strncmp(thisSerialNumber, gArgs.serialNumber, sizeof(thisSerialNumber))) {
                        LOG_DEBUG(("[%02d] matches desired serial number '%s'", i, thisSerialNumber));
                    } else {
                        LOG_DEBUG(("[%02d] does not match desired serial number '%s' (%s)", i, gArgs.serialNumber, thisSerialNumber));
                        potential_match = 0;
                    }
                } else {
                    LOG_DEBUG(("[%02d] has serial number '%s' (no match criteria provided)", i, thisSerialNumber));
                }
            } else {
                printf("[%02d] Error getting serial number: %s\n", i, get_error_string(error));
                potential_match = 0;
            }
        }

        // collect matching indices
        if (potential_match) {
            LOG_DEBUG(("[%02d] matched all search criteria", i));
            matching_devices[matching_device_count++] = i;
            if (gArgs.list) {
                printf("%02d %-*s %-*s\n", i, MAX_LABEL_SIZE, thisType, MAX_LABEL_SIZE, thisSerialNumber);
            }
        } else {
            LOG_DEBUG(("[%02d] failed to match search criteria", i));
        }
    }

    // give up if no matches
    if (!matching_device_count) {
        if (gArgs.list)
            puts("(no matching devices found)");
        cleanUpAndClose(!gArgs.list);
    }

    // reject invalid unary command combinations
    if (matching_device_count != 1) {
        if (gArgs.serialNumberNew) {
            printf("Error: can only update serial number on exactly one spectrometer (%d matched)\n",
                matching_device_count);
            cleanUpAndClose(1);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Process matching devices
    ////////////////////////////////////////////////////////////////////////////

    // process each matching device
    for (int i = 0; i < matching_device_count; i++) {
        int index = matching_devices[i];
        int error = 0;

        // get pixel count
        int pixels = seabreeze_get_formatted_spectrum_length(index, &error);
        LOG_DEBUG(("[%02d] has %d pixels", index, pixels));

        // --list-eeprom
        if (gArgs.listEEPROMs) {
            printf("[%02d] EEPROM slots 0..%d:\n", index, gArgs.listEEPROMs - 1);
            for (int j = 0; j < gArgs.listEEPROMs; j++) {
                char* errorMessage = NULL;
                unsigned char buf[MAX_LABEL_SIZE + 1];
                memset(buf, 0, sizeof(buf));

                // read value (both raw and as double)
                seabreeze_read_eeprom_slot(index, &error, j, buf, sizeof(buf));
                if (error) {
                    errorMessage = (char*) get_error_string(error);
                }

                double d = atof((const char*) buf);

                // report
                printf("[%02d]   %02d:", index, j);
                for (int k = 0; k < MAX_LABEL_SIZE; k++)
                    printf(" %02x", buf[k]);
                printf(" %+le [", d);
                for (int k = 0; k < MAX_LABEL_SIZE; k++)
                    printf("%c", buf[k] > 31 && buf[k] < 127 ? buf[k] : '.');
                printf("]");
                if (errorMessage)
                    printf(" (returned error: %s)", errorMessage);
                printf("\n");
                fflush(stdout);
            }
        }

        // --list-descriptors
        if (gArgs.listDescriptors) {
            printf("[%02d] USB Descriptors:\n", index);

            // Ocean Optics only uses descriptor strings 1 and 2, sometimes 3
            for (int j = 1; j < 3; j++)
            {
                int bytesCopied = 0;
                char* errorMessage = NULL;
                unsigned char buf[80];
                memset(buf, 0, sizeof(buf));

                bytesCopied = seabreeze_get_usb_descriptor_string(index, &error, j, buf, sizeof(buf));
                if (error) {
                    errorMessage = (char*) get_error_string(error);
                }

                // report
                printf("[%02d]   %02d:", index, j);
                if (errorMessage) {
                    printf(" (returned error %s)", errorMessage);
                } else if (bytesCopied) {
                    printf(" [%s]", (char*) buf);
                }
                printf("\n");
                fflush(stdout);
            }
        }

        // --list-irrad
        if (gArgs.listIrrad) {
            float *irrad = (float*) malloc(pixels * sizeof(float));
            int count = seabreeze_read_irrad_calibration(index, &error, irrad, pixels);
            fflush(stdout);
            if (!check_error(index, &error, "seabreeze_read_irrad_calibration")) {
                printf("[%02d] loaded %d irradiance calibration coefficients:\n", index, count);
                for (int j = 0; j < count; j++) {
                    printf("[%02d]   Coeff #%04d: %+le\n", index, j, irrad[j]);
                }
            }
            fflush(stdout);
            free(irrad);
        }

        // --list-edc-pixels
        if (gArgs.listEDCPixels) {
            #define MAX_INDICES 100
            int indices[MAX_INDICES];
            int count = seabreeze_get_electric_dark_pixel_indices(index, &error, indices, MAX_INDICES);
            fflush(stdout);
            if (!check_error(index, &error, "seabreeze_get_electric_dark_pixel_indices")) {
                printf("[%02d] found %d electrical dark pixel indices:\n", index, count);
                for (unsigned j = 0; j < count; j++) {
                    printf("[%02d]    EDC index #%04d: %d\n", index, j, indices[j]);
                }
            }
        }

        // --set-irrad
        if (gArgs.irradFilename) {
            FILE *infile = NULL;
            if ((infile = fopen(gArgs.irradFilename, "r")) != NULL) {
                float *irrad = (float*) malloc(pixels * sizeof(float));
                int count = 0;
                while(count < pixels && !feof(infile)) {
                    float f;
                    int ok = fscanf(infile, "%f", &f);
                    if (ok) {
                        irrad[count++] = f;
                    }
                }
                fclose(infile);
                printf("[%02d] read %d floats from %s\n", index, count, gArgs.irradFilename);
                if (count != pixels)
                    printf("[%02d] WARNING: irradiance count %d != device pixel count %d\n", index, count, pixels);
                printf("[%02d] writing %d floats to spectrometer (may take awhile)\n", index, count);
                fflush(stdout);
                int ok = seabreeze_write_irrad_calibration(index, &error, irrad, count);
                if (ok && !check_error(index, &error, "seabreeze_read_irrad_calibration")) {
                    printf("[%02d] wrote %d irradiance calibration coefficients to spectrometer\n",
                        index, count);
                }
                free(irrad);
            } else {
                printf("Error: can't read %s\n", gArgs.irradFilename);
            }
        }

        // --set-serial-number
        if (gArgs.serialNumberNew) {
            printf("[%02d] updating serial_number => [%s]\n", index, gArgs.serialNumberNew);
            int len = strlen(gArgs.serialNumberNew);
            unsigned char *buffer = malloc(len + 1);
            strncpy((char*) buffer, gArgs.serialNumberNew, len + 1);
            seabreeze_write_eeprom_slot(index, &error, 0, buffer, len + 1);
            free(buffer);
        }

        // --set-eeprom-value-hex
        if (gArgs.eepromValueHex) {
            printf("[%02d] setting eeprom slot %02d (0x%02x) ==> [%s] (bufferSize %d)\n", index, 
                gArgs.eepromIndex, gArgs.eepromIndex, gArgs.eepromValueHex, MAX_LABEL_SIZE);
            unsigned char *buffer = (unsigned char*) malloc(MAX_LABEL_SIZE);
            memset(buffer, 0, MAX_LABEL_SIZE);
            renderHex(buffer, MAX_LABEL_SIZE, gArgs.eepromValueHex);
            for (int byteIndex = 0; byteIndex < MAX_LABEL_SIZE; byteIndex++)
                printf("[%02d]   byte %02d: 0x%02x\n", index, byteIndex, buffer[byteIndex]);
            seabreeze_write_eeprom_slot(index, &error, gArgs.eepromIndex, buffer, MAX_LABEL_SIZE);
            free(buffer);
        }

        // --set-eeprom-value-ascii
        if (gArgs.eepromValueAscii) {
            printf("[%02d] setting eeprom slot %02d (0x%02x) ==> [%s] (bufferSize %d)\n", index, 
                gArgs.eepromIndex, gArgs.eepromIndex, gArgs.eepromValueAscii, MAX_LABEL_SIZE);
            unsigned char *buffer = (unsigned char*) malloc(MAX_LABEL_SIZE);
            memset(buffer, 0, MAX_LABEL_SIZE);
            memcpy(buffer, gArgs.eepromValueAscii, strlen(gArgs.eepromValueAscii));
            seabreeze_write_eeprom_slot(index, &error, gArgs.eepromIndex, buffer, MAX_LABEL_SIZE);
            free(buffer);
        }

        // --get-spectrum, --integration-time-us and --raw
        if (gArgs.getSpectrum)
        {
            double *wavelengths = (double*) malloc(pixels * sizeof(double));
            seabreeze_get_wavelengths(index, &error, wavelengths, pixels);
            seabreeze_set_integration_time_microsec(index, &error, gArgs.integrationTimeUS);
            if (gArgs.raw)
            {
                int bytes = seabreeze_get_unformatted_spectrum_length(index, &error);
                unsigned char* spectrum = (unsigned char*) malloc(bytes);
                if (spectrum != NULL)
                {
                    seabreeze_get_unformatted_spectrum(index, &error, spectrum, bytes); // throwaway for stabilization
                    seabreeze_get_unformatted_spectrum(index, &error, spectrum, bytes);
                    for (int i = 0; i < pixels; i++)
                    {
                        unsigned value = (spectrum[i * 2] & 0x00ff) | ((spectrum[i * 2 + 1] << 8) & 0x00ff00);
                        printf("%.2lf, %u\n", wavelengths[i], value);
                    }
                    free(spectrum);
                }
            }
            else
            {
                double* spectrum = (double*) malloc(pixels * sizeof(double));
                if (spectrum != NULL)
                {
                    seabreeze_get_formatted_spectrum(index, &error, spectrum, pixels); // throwaway for stabilization
                    seabreeze_get_formatted_spectrum(index, &error, spectrum, pixels);
                    for (int i = 0; i < pixels; i++)
                        printf("%.2lf, %.2lf\n", wavelengths[i], spectrum[i]);
                    free(spectrum);
                }
            }
        }

        // other future commands here...
    }

    // cleanup
    cleanUpAndClose(0);
}
