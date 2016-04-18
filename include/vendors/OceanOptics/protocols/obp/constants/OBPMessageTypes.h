/***************************************************//**
 * @file    OBPMessageTypes.h
 * @date    January 2011
 * @author  Ocean Optics, Inc.
 *
 * This class contains legal values for the messageType
 * field in the OBPMessage class.
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

#ifndef OBPMESSAGETYPES_H
#define OBPMESSAGETYPES_H

namespace seabreeze {
  namespace oceanBinaryProtocol {
    class OBPMessageTypes {
    public:
// Basic hardware and firmware information
//      static const unsigned int OBP_RESET                         = 0x00000000;
//      static const unsigned int OBP_RESET_DEFAULTS                = 0x00000001;
        static const unsigned int OBP_GET_HARDWARE_REVISION         = 0x00000080;
        static const unsigned int OBP_GET_FIRMWARE_REVISION         = 0x00000090;
        static const unsigned int OBP_GET_SERIAL_NUMBER             = 0x00000100;
        static const unsigned int OBP_GET_SERIAL_NUMBER_LENGTH      = 0x00000101;
        static const unsigned int OBP_GET_DEVICE_ALIAS              = 0x00000200;
        static const unsigned int OBP_GET_DEVICE_ALIAS_LENGTH       = 0x00000201;
        static const unsigned int OBP_SET_DEVICE_ALIAS              = 0x00000210;
        static const unsigned int OBP_GET_USER_STRING_COUNT         = 0x00000300;
        static const unsigned int OBP_GET_USER_STRING_LENGTH        = 0x00000301;
        static const unsigned int OBP_GET_USER_STRING               = 0x00000302;
        static const unsigned int OBP_SET_USER_STRING               = 0x00000310;

// Serial Interface
        static const unsigned int OBP_GET_RS232_BAUD_RATE           = 0x00000800;
        static const unsigned int OBP_GET_RS232_FLOW_COTROL_MODE    = 0x00000804;
        static const unsigned int OBP_SET_RS232_BAUD_RATE           = 0x00000810;
        static const unsigned int OBP_SET_RS232_FLOW_COTROL_MODE    = 0x00000814;
        static const unsigned int OBP_GET_RS232_SAVE_SETTINGS       = 0x000008F0;

// Miscellaneous
        static const unsigned int OBP_CONFIGURE_STATUS_LED          = 0x00001010;
//      static const unsigned int OBP_ENTER_REPROGRAMMING_MODE      = 0x000FFF00;


// Buffering
        static const unsigned int OBP_GET_BUFFER_SIZE_MAX           = 0x00100820;
        static const unsigned int OBP_GET_BUFFER_SIZE_ACTIVE        = 0x00100822;
        static const unsigned int OBP_CLEAR_BUFFER_ALL              = 0x00100830;
        static const unsigned int OBP_SET_BUFFER_SIZE_ACTIVE        = 0x00100832;
        static const unsigned int OBP_GET_BUFFERED_SPEC_COUNT       = 0x00100900;

//Spectra queries
        static const unsigned int OBP_GET_BUF_SPEC32_META           = 0x00100928;
        static const unsigned int OBP_GET_CORRECTED_SPECTRUM_NOW    = 0x00101000;
        static const unsigned int OBP_GET_SATURATION_LEVEL          = 0x001010A0;
        static const unsigned int OBP_GET_RAW_SPECTRUM_NOW          = 0x00101100;
        static const unsigned int OBP_GET_PARTIAL_SPECTRUM_MODE     = 0x00102000;
        static const unsigned int OBP_SET_PARTIAL_SPECTRUM_MODE     = 0x00102010;
        static const unsigned int OBP_GET_PARTIAL_SPECTRUM_NOW      = 0x00102080;

// Integration, binning, lamps, shutters etc.
        static const unsigned int OBP_SET_ITIME_USEC                = 0x00110010;
        static const unsigned int OBP_SET_TRIG_MODE                 = 0x00110110;
        static const unsigned int OBP_SIMULATE_TRIG_PULSE           = 0x00110120;
        static const unsigned int OBP_GET_PIXEL_BINNING_FACTOR      = 0x00110280;
        static const unsigned int OBP_GET_MAX_BINNING_FACTOR        = 0x00110281;
        static const unsigned int OBP_GET_DEFAULT_BINNING_FACTOR    = 0x00110285;
        static const unsigned int OBP_SET_PIXEL_BINNING_FACTOR      = 0x00110290;
        static const unsigned int OBP_SET_DEFAULT_BINNING_FACTOR    = 0x00110295;
        static const unsigned int OBP_SET_LAMP_ENABLE               = 0x00110410;
        static const unsigned int OBP_SET_TRIG_DELAY_USEC           = 0x00110510;
        static const unsigned int OBP_SET_SHUTTER                   = 0x00110610;

// Spectrum filtering and averaging
        static const unsigned int OBP_GET_SCANS_TO_AVERAGE          = 0x00120000;
        static const unsigned int OBP_SET_SCANS_TO_AVERAGE          = 0x00120010;
        static const unsigned int OBP_GET_BOXCAR_WIDTH              = 0x00121000;
        static const unsigned int OBP_SET_BOXCAR_WIDTH              = 0x00121010;

// Spectral correction and calibration
        static const unsigned int OBP_GET_WL_COEFF_COUNT            = 0x00180100;
        static const unsigned int OBP_GET_WL_COEFF                  = 0x00180101;
        static const unsigned int OBP_SET_WL_COEFF                  = 0x00180111;
        static const unsigned int OBP_GET_NL_COEFF_COUNT            = 0x00181100;
        static const unsigned int OBP_GET_NL_COEFF                  = 0x00181101;
        static const unsigned int OBP_SET_NL_COEFF                  = 0x00181111;
        static const unsigned int OBP_GET_IRRAD_CAL_ALL             = 0x00182001;
        static const unsigned int OBP_GET_IRRAD_CAL_COUNT           = 0x00182002;
        static const unsigned int OBP_GET_IRRAD_CAL_COLL_AREA       = 0x00182003;
        static const unsigned int OBP_SET_IRRAD_CAL_ALL             = 0x00182011;
        static const unsigned int OBP_SET_IRRAD_CAL_COLL_AREA       = 0x00182013;
        static const unsigned int OBP_GET_STRAY_COEFF_COUNT         = 0x00183100;
        static const unsigned int OBP_GET_STRAY_COEFF               = 0x00183101;
        static const unsigned int OBP_SET_STRAY_COEFF               = 0x00183111;
        static const unsigned int OBP_SPEC_GET_HOT_PIXEL_INDICES    = 0x00186000;
        static const unsigned int OBP_SPEC_SET_HOT_PIXEL_INDICES    = 0x00186010;

// Optical Bench
        static const unsigned int OBP_GET_BENCH_ID                  = 0x001B0000;
        static const unsigned int OBP_GET_BENCH_SERIAL_NUMBER       = 0x001B0100;
        static const unsigned int OBP_GET_BENCH_SLIT_WIDTH_MICRONS  = 0x001B0200;
        static const unsigned int OBP_GET_BENCH_FIBER_DIAM_MICRONS  = 0x001B0300;
        static const unsigned int OBP_GET_BENCH_GRATING             = 0x001B0400;
        static const unsigned int OBP_GET_BENCH_FILTER              = 0x001B0500;
        static const unsigned int OBP_GET_BENCH_COATING             = 0x001B0600;

// GPIO
        static const unsigned int OBP_GET_GPIO_NUMBER_OF_PINS       = 0x00200000;
        static const unsigned int OBP_GET_GPIO_OUTPUT_ENABLE_VECTOR = 0x00200100;
        static const unsigned int OBP_SET_GPIO_OUTPUT_ENABLE_VECTOR = 0x00200110;
        static const unsigned int OBP_GET_GPIO_VALUE                = 0x00200300;
        static const unsigned int OBP_SET_GPIO_VALUE                = 0x00200310;

// Strobe
        static const unsigned int OBP_SET_SINGLE_STROBE_DELAY       = 0x00300010;
        static const unsigned int OBP_SET_SINGLE_STROBE_WIDTH       = 0x00300011;
        static const unsigned int OBP_SET_SINGLE_STROBE_ENABLE      = 0x00300012;
        static const unsigned int OBP_SET_CONT_STROBE_PERIOD        = 0x00310010;
        static const unsigned int OBP_SET_CONT_STROBE_ENABLE        = 0x00310011;

// Temperature control and sensing
        static const unsigned int OBP_GET_TEMPERATURE_COUNT         = 0x00400000;
        static const unsigned int OBP_GET_TEMPERATURE               = 0x00400001;
        static const unsigned int OBP_GET_TEMPERATURE_ALL           = 0x00400002;
        static const unsigned int OBP_GET_TE_ENABLE                 = 0x00420000;
        static const unsigned int OBP_GET_TE_SETPOINT               = 0x00420001;
        static const unsigned int OBP_GET_TE_FAN_ENABLE             = 0x00420002;
        static const unsigned int OBP_GET_TE_IS_STABLE              = 0x00420003;
        static const unsigned int OBP_GET_TE_TEMPERATURE            = 0x00420004;
        static const unsigned int OBP_SET_TE_ENABLE                 = 0x00420010;
        static const unsigned int OBP_SET_TE_SETPOINT               = 0x00420011;
        static const unsigned int OBP_SET_TE_FAN_ENABLE             = 0x00420012;

// light source control
        static const unsigned int OBP_GET_LIGHT_SOURCE_ENABLE       = 0x00810021;
        static const unsigned int OBP_SET_LIGHT_SOURCE_ENABLE       = 0x00810031;
        static const unsigned int OBP_GET_LIGHT_SOURCE_INTENSITY    = 0x00810041;
        static const unsigned int OBP_SET_LIGHT_SOURCE_INTENSITY    = 0x00810051;
    };
  }
}

#endif /* OBPMESSAGETYPES_H */

