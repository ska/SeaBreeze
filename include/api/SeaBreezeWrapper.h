/***************************************************//**
 * @file    SeaBreezeWrapper.h
 * @date    July 2009
 * @author  Ocean Optics, Inc.
 *
 * This is a trivial interface to SeaBreeze that allows
 * the user to connect to devices over USB.
 * This is intended as a usable and extensible API.
 *
 * This provides a C interface to help with linkage.
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

#ifndef SEABREEZE_WRAPPER_H
#define SEABREEZE_WRAPPER_H

#define SEABREEZE_API_VERSION "3.0.11"  //!< current version of the SeaBreezeWrapper API
#define SEABREEZE_MAX_DEVICES 32        //!< how many different spectrometer types we support

#include "api/DllDecl.h"

#ifdef __cplusplus

/**
* @brief Encapsulates all SeaBreeze classes
*/
namespace seabreeze {
	class Device;
}

/**
    Original interface to SeaBreeze.  SeaBreezeAPI represents a proposed architecture for
    future development.
*/
class DLL_DECL SeaBreezeWrapper {
public:

    //! get handle to Singleton
    static SeaBreezeWrapper *getInstance();

    //! free some static memory under Microsoft profiler
    static void shutdown();

    // lifecycle
    int    openSpectrometer          (int index, int *errorCode);
    int    closeSpectrometer         (int index, int *errorCode);

    // metadata
    int    getModel                  (int index, int *errorCode, char *buf, int len);
    int    getSerialNumber           (int index, int *errorCode, char *buf, int len);

    // basic acquisitions
    void   setIntegrationTimeMicrosec(int index, int *errorCode, unsigned long integration_time_micros);
    int    getWavelengths            (int index, int *errorCode, double *wavelengths, int length);
    int    getFormattedSpectrum      (int index, int *errorCode, double* buffer, int buffer_length);
    int    getUnformattedSpectrum    (int index, int *errorCode, unsigned char *buffer, int buffer_length);
    int    getFormattedSpectrumLength(int index, int *errorCode);
    int    getUnformattedSpectrumLength(int index, int *errorCode);
    long   getMinIntegrationTimeMicrosec(int index, int *errorCode);
    long   getMaxIntegrationTimeMicrosec(int index, int *errorCode);
    int    getMaximumIntensity(int index, int *errorCode);

    // advanced features
    int    getElectricDarkPixelIndices(int index, int *errorCode, int *indices, int length);
    void   setTriggerMode            (int index, int *errorCode, int mode);
    void   setStrobeEnable           (int index, int *errorCode, unsigned char strobe_enable);
    void   setShutterOpen            (int index, int *errorCode, unsigned char opened);
    void   setContinuousStrobePeriodMicrosec(int index, int *errorCode, unsigned short strobe_id, unsigned long period_usec);
    void   setAcquisitionDelayMicrosec(int index, int *errorCode, unsigned long delay_usec);

    // Buffering features
    void   clearBuffer               (int index, int *errorCode);
    unsigned long getBufferElementCount(int index, int *errorCode);
    unsigned long getBufferCapacity  (int index, int *errorCode);
    unsigned long getBufferCapacityMaximum(int index, int *errorCode);
    unsigned long getBufferCapacityMinimum(int index, int *errorCode);
    void   setBufferCapacity         (int index, int *errorCode, unsigned long capacity);

    // EEPROM access
    int    readEEPROMSlot            (int index, int *errorCode, int slot_number, unsigned char *buffer, int buffer_length);
    int    writeEEPROMSlot           (int index, int *errorCode, int slot_number, unsigned char *buffer, int buffer_length);

    // irradiance calibration
    int    readIrradCalibration      (int index, int *errorCode, float *buffer, int buffer_length);
    int    writeIrradCalibration     (int index, int *errorCode, float *buffer, int buffer_length);
    int    hasIrradCollectionArea    (int index, int *errorCode);
    float  readIrradCollectionArea   (int index, int *errorCode);
    void   writeIrradCollectionArea  (int index, int *errorCode, float area);

    // thermal-electric cooler
    double readTECTemperature        (int index, int *errorCode);
    void   setTECTemperature         (int index, int *errorCode, double temperature_degrees_celsius);
    void   setTECEnable              (int index, int *errorCode, unsigned char tec_enable);
    void   setTECFanEnable           (int index, int *errorCode, unsigned char tec_fan_enable);

    // Wrapper features
    void   setVerbose                (bool flag);
    void   setLogfile                (char *path, int length);
    int    getAPIVersionString       (char *buffer, int length);
    int    getErrorString            (int errorCode, char *buffer, int buffer_length);

    // raw USB access
    int    writeUSB                  (int index, int *errorCode, unsigned char endpoint, unsigned char *buffer, unsigned int length);
    int    readUSB                   (int index, int *errorCode, unsigned char endpoint, unsigned char *buffer, unsigned int length);
    int    getUSBDescriptorString    (int index, int *errorCode, int id, unsigned char *buffer, int length);

    // light sources (JAZ?)
    int    getLightSourceCount       (int index, int *errorCode);
    void   setLightSourceEnable      (int index, int *errorCode, int lightIndex, unsigned char enable);
    void   setLightSourceIntensity   (int index, int *errorCode, int lightIndex, double intensity);

private:
    SeaBreezeWrapper();
    virtual ~SeaBreezeWrapper();

    //! Singleton
    static SeaBreezeWrapper *instance;

    //! types of supported Ocean Optics devices (not actual devices found enumerated on the USB bus)
    seabreeze::Device *devices[SEABREEZE_MAX_DEVICES];
};

extern "C" {
#endif /* __cplusplus */

    /* All of these C functions start with seabreeze_ to prevent namespace
     * collisions.
     */

    /**
     * @brief This function opens a device attached to the system.
     * @param index (Input) The index of a USB device to try to open.
     *        Valid values will range from 0 to N-1 for N connected devices.
     * @param error_code (Output) A pointer to an integer that can be used
     *        for storing error codes.
     * @return int: The function will return an integer of 0 if it opened a
     *      device successfully, or 1 if no device was opened (in which
     *      case the error_code variable will be set).
     *
     * This can be called repeatedly with incrementing index values (until
     * it returns 1) to open all connected devices.
     *
     * Note that the index used to open a device with this function should also
     * be used to communicate with that same device in the other functions
     * provided here.
     */
    DLL_DECL int
    seabreeze_open_spectrometer(int index, int *error_code);

    /**
     * @brief This function closes the spectrometer attached to the system.
     * @param index (Input) The index of a device previously opened with
     *        open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used
     *        for storing error codes.
     * @return int: This function will return 1 no matter what!  (MZ)
     */
    DLL_DECL int
    seabreeze_close_spectrometer(int index, int *error_code);

    /**
     * @brief This function returns a description of the error denoted by
     *        error_code.
     * @param error_code (Input) The integer error code to look up.  Error
     *        codes not be zero, but can be any non-zero integer (positive or
     *        negative).
     * @param buffer (Output) A character buffer allocated to contain at least
     *        'buffer_length' bytes, which will be populated with the string
     *        description of the given error code.
     * @param buffer_length (Input) allocated size of the output buffer
     * @return int: Number of bytes written to buffer.
     */
    DLL_DECL int
    seabreeze_get_error_string(int error_code, char *buffer, int buffer_length);

    /**
     * @brief This function returns a string denoting the type of the device.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *      error codes.  This may be NULL.
     * @param buffer (Output)  A character buffer allocated to contain at least
     *      'buffer_length' bytes, which will be populated with the
     *      spectrometer type.
     * @param buffer_length (Input) allocated size of the buffer
     * @return int: Number of bytes written to the buffer.
     *
     * The populated buffer will hold one of the following strings:
     *
     * \verbatim
     *      NONE:        Used if no spectrometer is found (error_code will also be set)
     *      HR2000:      Represents an HR2000 Spectrometer
     *      HR2000PLUS:  Represents an HR2000+ Spectrometer
     *      HR4000:      Represents an HR4000 Spectrometer
     *      JAZ:         Represents a Jaz Spectrometer
     *      MAYA2000:    Represents a Maya2000 Spectrometer
     *      MAYALSL:     Represents a Maya-LSL Spectrometer
     *      MAYA2000PRO: Represents a Maya2000 Pro Spectrometer
     *      NIRQUEST256: Represents an NIRQuest256 Spectrometer
     *      NIRQUEST512: Represents an NIRQuest512 Spectrometer
     *      QE65000:     Represents a QE65000 Spectrometer
     *      QE-PRO:      Represents a QE-Pro Spectrometer
     *      STS:         Represents an STS Spectrometer
     *      TORUS:       Represents a Torus Spectrometer
     *      USB2000:     Represents a USB2000 Spectrometer
     *      USB2000PLUS: Represents a USB2000+ Spectrometer
     *      USB4000:     Represents a USB4000 Spectrometer
     * \endverbatim
     */
    DLL_DECL int
    seabreeze_get_model(int index, int *error_code, char *buffer, int buffer_length);

    /**
     * @brief This function sets the trigger mode for the specified device.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *      error codes.
     * @param mode (Input) a trigger mode (0 = normal, 1 = software, 2 = synchronization,
     *        3 = external hardware, etc.)
     *
     * Note that requesting an unsupported mode will result in an error.
     */

    DLL_DECL void
    seabreeze_set_trigger_mode(int index, int *error_code, int mode);

    /**
     * @brief This function sets the integration time for the specified device.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *      error codes.
     * @param integration_time_micros (Input) The new integration time in units of
     *      microseconds
     *
     * This function does not automatically perform a stability scan.
     * If your application requires a stability scan following a change
     * in integration time, you need to command that yourself.
     */
    DLL_DECL void
    seabreeze_set_integration_time_microsec(int index, int *error_code,
            unsigned long integration_time_micros);

    /**
     * @brief This function returns the smallest integration time setting,
     *        in microseconds, that is valid for the spectrometer.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @return Returns minimum legal integration time in microseconds if > 0.
     *        On error, returns -1 and error_code will be set accordingly.
     */
    DLL_DECL long
    seabreeze_get_min_integration_time_microsec(int index, int *error_code);

    /**
     * @brief This function sets the shutter state on the spectrometer.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param opened (Input) A logical boolean used for denoting the desired
     *      state (opened/closed) of the shutter.   If the value of opened is
     *      non-zero, then the shutter will open.  If the value of opened is
     *      zero, then the shutter will close.
     */
    DLL_DECL void
    seabreeze_set_shutter_open(int index, int *error_code, unsigned char opened);

    /**
     * @brief This function sets the strobe enable on the spectrometer.  Note that
     *        this refers to a particular set of one or more digital pins on the
     *        device: lamp enable, single strobe, and continuous strobe may all
     *        be affected by this setting, and these generally control lamps
     *        that are external to the device.  Note that this is related to, but
     *        different from, the light source interface which allows the intensity
     *        and/or enable status of individual light sources (e.g. lamp bulbs,
     *        LEDs, or lasers) in attached modules to be controlled.  Refer to
     *        the seabreeze_xxx_light_source_ functions for finer control of
     *        certain light source modules that are more closely integrated with
     *        the spectrometer.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output)  A pointer to an integer that can be used for storing
     *        error codes.
     * @param strobe_enable (Input) A logical boolean used for denoting the
     *      desired value (high/low) of the strobe-enable pin.   If the value of
     *      strobe_enable is zero, then the pin should be set low.  If the value
     *      of strobe_enable is non-zero, then the pin should be set high.
     */
    DLL_DECL void
    seabreeze_set_strobe_enable(int index, int *error_code, unsigned char strobe_enable);

    /**
     * @brief This function gets the number of attached light sources that can
     *        be programmatically controlled.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output)  A pointer to an integer that can be used for storing
     *        error codes.
     * @return The number of light sources that can be controlled
     */
    DLL_DECL int
    seabreeze_get_light_source_count(int index, int *error_code);

    /**
     * @brief This function sets the enable status on a connected light source.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output)  A pointer to an integer that can be used for storing
     *        error codes.
     * @param light_index (Input)  The index of the light source.  This will
     *      usually be zero, but if the light module contains multiple LEDs,
     *      bulbs, lasers, etc. then this may be higher.  Use
     *      seabreeze_get_light_source_count() to bound the maximum value .
     * @param enable (Input) A logical boolean used for denoting whether to enable
     *      the indicated light source.   If the value of
     *      enable is zero, then the light source should be disabled.  If the value
     *      of enable is non-zero, then the light source should be enabled.
     */
    DLL_DECL void
    seabreeze_set_light_source_enable(int index, int *error_code,
            int light_index, unsigned char enable);

    /**
     * @brief This function sets the intensity of a connected light source.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output)  A pointer to an integer that can be used for storing
     *        error codes.
     * @param light_index (Input)  The index of the light source.  This will
     *      usually be zero, but if the light module contains multiple LEDs,
     *      bulbs, lasers, etc. then this may be higher.  Use
     *      seabreeze_get_light_source_count() to bound the maximum value.
     * @param intensity (Input) The desired intensity of the light source.  The
     *      range of intensities is normalized over [0, 1], where 0 is the
     *      minimum controllable intensity of the light source, and 1 is the
     *      maximum.
     *
     * @warning SETTING THE INTENSITY TO ZERO MAY NOT CAUSE THE LIGHT SOURCE
     *      TO TURN OFF COMPLETELY.  The light source will go to the
     *      dimmest level it can reach without changing its enable status.
     *      To switch the light source off, try using the
     *      seabreeze_set_light_source_enable() function or provide the
     *      operator with another way to disable or block the light source.
     */
    DLL_DECL void
    seabreeze_set_light_source_intensity(int index, int *error_code,
            int light_index, double intensity);

    /**
     * @brief This function reads a string out of the spectrometer's EEPROM slot
     *        and returns the result.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param slot_number (Input) The number of the slot to read out.  Possible
     *        values are 0 through 16*.
     * @param buffer (Output)  A buffer (with memory already allocated) to hold the
     *        value read out of the EEPROM slot
     * @param buffer_length (Input) The length of the allocated output buffer (typically 16)
     * @return int: The number of bytes read from the EEPROM slot into the buffer
     *
     * (*) Actual maximum slot count varies by spectrometer model.  For the number of supported
     *     slots on your spectrometer, see EEPROMSlotFeature instantiation in appropriate
     *     device file under src/vendors/OceanOptics/devices.
     */
    DLL_DECL int
    seabreeze_read_eeprom_slot(int index, int *error_code, int slot_number, unsigned char *buffer,
            int buffer_length);

    /**
     * @brief This function writes a string to a spectrometer's EEPROM slot
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer for storing error codes.
     * @param slot_number (Input) The number of the slot being written (e.g. 0-16*).
     * @param buffer (Input) A buffer containing the text to write to the slot.
     * @param buffer_length (Input) The length of the text to write (typically 15)
     * @return number of bytes written
     *
     * @warning CARELESS USE OF THIS FUNCTION CAN RENDER YOUR SPECTROMETER INOPERABLE,
     *          POSSIBLY REQUIRING R.M.A. OR REFLASHING TO RESTORE FUNCTIONALITY.
     *          PLEASE READ YOUR SPECTROMETER'S DATA SHEET CAREFULLY BEFORE USE.
     *
     * (*) See note in seabreeze_read_eeprom_slot() regarding per-device slot limits.
     */
    DLL_DECL int
    seabreeze_write_eeprom_slot(int index, int *error_code, int slot_number, unsigned char *buffer,
            int buffer_length);

    /**
     * @brief This function reads out an irradiance calibration from the spectrometer's
     *        internal memory if that feature is supported.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param buffer (Output) array of floating point values into which calibration values are stored
     * @param buffer_length (Input) maximum number of values to copy from the device into buffer
     * @return int: the number of floats read from the device into the buffer
     */
    DLL_DECL int
    seabreeze_read_irrad_calibration(int index, int *error_code, float *buffer,
        int buffer_length);

    /**
     * @brief This function writes an irradiance calibration to the spectrometer's
     *        internal memory if that feature is supported.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param buffer (Output) array of floating point values to store into the device
     * @param buffer_length (Input) number of calibration factors to write
     * @return int: the number of floats written from the buffer to the device
     */
    DLL_DECL int
    seabreeze_write_irrad_calibration(int index, int *error_code, float *buffer,
        int buffer_length);

    /**
     * @brief This function checks for an irradiance collection area in the spectrometer's
     *        internal memory if that feature is supported.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @return int: 0 if no collection area available, 1 if available.
     */
    DLL_DECL int
    seabreeze_has_irrad_collection_area(int index, int *error_code);

    /**
     * @brief This function reads an irradiance collection area from the spectrometer's
     *        internal memory if that feature is supported.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @return float: collection area (typically in units of cm^2) read from device
     */
    DLL_DECL float
    seabreeze_read_irrad_collection_area(int index, int *error_code);

    /**
     * @brief This function writes an irradiance collection area to the spectrometer's
     *        internal memory if that feature is supported.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param area (Input) collection area to save to the EEPROM (presumably cm^2)
     */
    DLL_DECL void
    seabreeze_write_irrad_collection_area(int index, int *error_code, float area);

    /**
     * @brief This function reads the value of the TEC and returns the value in
     *        degrees celsius.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @return int: The TEC temperature in degrees Celsius.
     */
    DLL_DECL double
    seabreeze_read_tec_temperature(int index, int *error_code);

    /**
     * @brief This function sets the TEC temperature.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param temperature_degrees_celsius (Input) The desired temperature, in degrees
     *        Celsius.
     */
    DLL_DECL void
    seabreeze_set_tec_temperature(int index, int *error_code,
            double temperature_degrees_celsius);

    /**
     * @brief This function enables the TEC feature on the spectrometer.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param tec_enable (Input) A logical boolean that denotes the desired TEC enable
     *        state.  If the value of tec_enable is zero, the TEC should be disabled.
     *        If the value of tec_enable is non-zero, the TEC should be enabled.
     */
    DLL_DECL void
    seabreeze_set_tec_enable(int index, int *error_code, unsigned char tec_enable);

    /**
     * @brief This function enables the TEC Fan on the spectrometer.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param tec_fan_enable (Input) A logical boolean that denotes the desired TEC fan enable
     *        state.  If the value of tec_fan_enable is zero, the TEC fan should be disabled.
     *        If the value of tec_fan_enable is non-zero, the TEC fan should be enabled.
     */
    DLL_DECL void
    seabreeze_set_tec_fan_enable(int index, int *error_code, unsigned char tec_fan_enable);

    /**
     * @brief This acquires a spectrum and returns the answer in raw, unformatted bytes.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param buffer (Output) A buffer (with memory already allocated) to hold the
     *        spectral data
     * @param buffer_length (Input) The length of the buffer in bytes (not pixels)
     * @return int: The number of bytes read into the buffer
     * @see   sample-code/c/demo-unformatted-spectrum.c
     *
     * An unformatted spectrum is the raw sequence of bytes returned by the spectrometer to 
     * the PC over USB.  The bytes have not been broken down into pixels, they're in the 
     * original endianness, absolutely nothing has been done to them.  The caller is expected 
     * to know how many bytes are returned by each spectrometer model, which bytes indicate 
     * synchronization points or whatever, etc.  
     */
    DLL_DECL int
    seabreeze_get_unformatted_spectrum(int index, int *error_code,
            unsigned char *buffer, int buffer_length);

    /**
     * @brief This acquires a spectrum and returns the answer in formatted
     *        floats.  In this mode, auto-nulling should be automatically
     *        performed for devices that support it.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param buffer (Output) A buffer (with memory already allocated) to hold the
     *        spectral data
     * @param buffer_length (Input) The length of the buffer in floats (not bytes)
     * @return int: The number of floats read into the buffer
     *
     * A formatted spectrum returns exactly one double-precision floating-point IEEE value 
     * per pixel, as opposed to a raw byte stream.  It has also had autonulling (gain control) 
     * applied, meaning it has been scaled up to the spectrometer's full dynamic range using 
     * the gain setting recorded in that spectrometer’s EEPROM.
     */
    DLL_DECL int
    seabreeze_get_formatted_spectrum(int index, int *error_code,
            double* buffer, int buffer_length);

    /**
     * @brief This returns an integer denoting the length of a raw spectrum
     *        (as returned by get_unformatted_spectrum(...)).
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *      error codes.
     * @return int: An integer denoting the length of an unformatted spectrum in bytes
     *
     * The caller is expected to know the number of bytes per pixel and the endian
     * ordering, but it will normally be 2 bytes per pixel, LSB-MSB order.
     */
    DLL_DECL int
    seabreeze_get_unformatted_spectrum_length(int index, int *error_code);

    /**
     * @brief This returns an integer denoting the number of pixels in a
     *        formatted spectrum (as returned by get_formatted_spectrum(...)).
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @return int: An integer denoting the length of a formatted spectrum (in pixels)
     */
    DLL_DECL int
    seabreeze_get_formatted_spectrum_length(int index, int *error_code);

    /**
     * @brief This computes the wavelengths for the spectrometer and fills in the
     *        provided array (up to the given length) with those values.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param wavelengths (Output) A pre-allocated array of doubles into which the
     *        wavelengths will be copied
     * @param length (Input) The number of values to copy into the wavelength array (this should
     *        be no larger than the number of doubles allocated in the wavelengths
     *        array)
     * @return int: An integer denoting the number of wavelengths written to the buffer
     */
    DLL_DECL int
    seabreeze_get_wavelengths(int index, int *error_code, double *wavelengths, int length);

    /**
     * @brief This reads the device's serial number and fills the
     *        provided array (up to the given length) with it.
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param buffer (Output) A pre-allocated array of characters into which the serial number
     *        will be copied
     * @param buffer_length (Input) The number of values to copy into the buffer (this should
     *        be no larger than the number of chars allocated in the buffer) (typically 16)
     * @return int: An integer denoting the number of bytes written into the buffer
     *
     * Note that "serial numbers" may include both digits and letters
     */
    DLL_DECL int
    seabreeze_get_serial_number(int index, int *error_code, char *buffer, int buffer_length);

    /**
     * @brief This fills in the provided array (up to the given length) with the indices
     *        of the pixels that are electrically active but optically masked
     *        (a.k.a. electric dark pixels).
     * @param index (Input) The index of a device previously opened with open_spectrometer().
     * @param error_code (Output) A pointer to an integer that can be used for storing
     *        error codes.
     * @param indices (Output) A pre-allocated array of ints into which the pixel indices
     *        will be copied
     * @param length (Input) The number of values to copy into the indices array (this should
     *        be no larger than the number of ints allocated in the indices array)
     * @return int: An integer denoting the number of indices written into the indices buffer
     *
     * Note that not all detectors have optically masked pixels; in that case,
     * this function will return zero.
     */
    DLL_DECL int
    seabreeze_get_electric_dark_pixel_indices(int index, int *error_code,
        int *indices, int length);

    /**
     * @brief Shutdown SeaBreeze completely, releasing all resources and destroying
     *        any cached device handles.
     *
     * This function is not normally needed (Singletons are destroyed automatically
     * at process end), but calling this explicitly can resolve some spurious warnings
     * in highly paranoid memory leak profilers.
     */
    DLL_DECL void
    seabreeze_shutdown();

    /**
    * @brief Write a raw array of bytes to a USB spectrometer.
    * @param index (Input) index of an opened spectrometer
    * @param errorCode (Output) pointer to an allocated integer field for receiving error codes
    * @param endpoint (Input) USB endpoint for write operation
    *        (see src/vendors/OceanOptics/features/raw_bus_access/RawUSBBusAccessFeature.cpp)
    * @param buffer (Input) array of bytes to send to USB spectrometer
    * @param length (Input) number of bytes to write from buffer to spectrometer
    * @return number of bytes written
    *
    * Write the bytes in the buffer according to the two command bytes at the
    * start of the buffer.
    *
    * \section Endpoints
    *
    * You may wonder why the caller needs to specify an explicit endpoint for
    * raw read and write operations; does not SeaBreeze already know the appro-
    * priate endpoints for any open spectrometer, for example.  Yes it does,
    * but currently there is no easy way to extract endpoints from a generic
    * spectrometer, due to the fact that OOIUSBCypressEndpointMap and
    * OOIUSBSTSEndpointMap do not share a common base class (see
    * OOIUSBEndpointMaps.h).
    *
    * We could attempt to extract them from the TransferHelpers held by
    * OOIUSBInterface, but without a Hint to use as a key, we would likely
    * obtain unmatched or inappropriate versions for control exchanges.
    * As ControlHint and OBPControlHint share no common ancestor or type data
    * to associate them while distinguishing from other hint types, we cannot
    * automatically infer appropriate control endpoints.
    *
    * And finally, we have no way of knowing that the user really wants to
    * use control endpoints anyway: they may actually desire high-speed
    * spectral retrieval.  So really, this needs to remain a manual external
    * operation until such time as we provide a much richer, generic query
    * interface over our internal endpoint lookup tables.
    *
    * \section Example
    *
    * For instance, for most USB spectrometers, the hex values <tt>05 00</tt> in
    * the buffer means 'read EEPROM slot 0'. Writing this command, and following
    * it with the \c seabreeze_read_usb command and the same hex <tt>05 00</tt>
    * in the buffer, will result in the spectrometer's serial number, in ASCII
    * characters, being delivered to the subsequent bytes in the buffer array.
    * This would appear in the output buffer as:
    *
    * \code
    *   05 00 55 53 42 32 2B 48 30 31 34 31 36 00 00 00 00 (hex)
    * \endcode
    *
    * (<tt>USB2+H01416</tt> ASCII, skipping the two command bytes)
    *
    * A C or C++ program could use the data as-is, but a C# program could
    * append to a string, each byte, cast as char, stopping on the first null
    * character:
    *
    * \code
    *   string serial = "";
    *   for (int i = 0; i < length && buffer[i] != '\0'; i++)
    *     serial += (char)buffer[i];
    * \endcode
    *
    * This is equivalent to:
    *
    * \code
    *   string serial = System.Text.ASCIIEncoding.ASCII.GetString(buffer).Trim('\0');
    * \endcode
    *
    * For C# and VB.NET, the trailing zero-fill must be removed. Those null
    * characters would not print or display on a screen but would make a file
    * name invalid.
    */
    DLL_DECL int
    seabreeze_write_usb(int index, int *errorCode, unsigned char endpoint, unsigned char* buffer, unsigned int length);

    /**
    * @brief Read a raw array of bytes from a USB spectrometer.
    * @param index (Input) index of a previously opened spectrometer
    * @param errorCode (Output) pointer to an allocated integer field for receiving error codes
    * @param endpoint (Input) USB endpoint for read operation
    *        (see src/vendors/OceanOptics/features/raw_bus_access/RawUSBBusAccessFeature.cpp)
    * @param buffer (Input) array of allocated bytes at which to recieve USB data
    * @param length (Input) maximum number of bytes to read from spectrometer
    * @return number of received bytes written to buffer
    *
    * Read bytes from the spectrometer into the buffer. The buffer starts with
    * two bytes of command information that will be followed by the transferred
    * bytes. The read and write USB functions allow low-level control of the
    * spectrometer via USB commands. The higher-level SeaBreeze functions issue
    * USB commands internally.
    *
    * Reading USB data takes two steps. First a seabreeze_write_usb call requests
    * the transfer, then a seabreeze_read_usb call delivers the data. Writing a buffer
    * to USB takes one step. The operating system handle for the spectrometer is
    * managed by SeaBreeze. USB has a concept of "end points". These are implicitly
    * addressed by the seabreeze_read_usb and seabreeze_write_usb functions.
    */
    DLL_DECL int
    seabreeze_read_usb(int index, int *errorCode, unsigned char endpoint, unsigned char* buffer, unsigned int length);

    /**
    * @brief Get the SeaBreeze library's internal version identifier.
    * @param buffer (Output) pointer to an allocated character array
    *                        to hold the returned version string
    * @param len    (Input)  size of the allocated buffer
    * @return number of bytes written to buffer
    */
    DLL_DECL int
    seabreeze_get_api_version_string(char *buffer, int len);

    /**
    * @brief Get a USB descriptor string by number
    * @param index     (Input)  Which spectrometer to set
    * @param errorCode (Output) pointer to allocated integer to receive error code
    * @param id        (Input)  numeric ID of the desired USB descriptor string
    * @param buffer    (Output) pointer to an allocated character array to hold
    *                           the returned descriptor string
    * @param len       (Input)  size of the allocated buffer
    * @return number of bytes written to buffer
    */
    DLL_DECL int
    seabreeze_get_usb_descriptor_string(int index, int *errorCode, int id, unsigned char *buffer, int len);

    /**
    * @brief Set the continuous strobe period in microseconds
    * @param index (Input) Which spectrometer to set
    * @param errorCode (Output) pointer to allocated integer to receive error code
    * @param strobe_id (Input) index of the strobe generator (currently always zero)
    * @param period_usec (Input) total period of the strobe, in microseconds
    * @return zero (on success or failure; check errorCode)
    *
    * The resolution is 0.1 milliseconds (100 microseconds).
    */
    DLL_DECL void
    seabreeze_set_continuous_strobe_period_microsec(int index, int *errorCode,
        unsigned short strobe_id, unsigned long period_usec);

    /**
    * @brief Set the acquisition delay (trigger delay) in microseconds.  This
    *        controls the amount of time between a particular event
    *        (usually a request for spectrum or an external trigger pulse)
    *        and the start of acquisition.
    */
    DLL_DECL void
    seabreeze_set_acquisition_delay_microsec(int index,
        int *errorCode, unsigned long delay_usec);

    /**
    * @brief Clear the spectrum buffer (if equipped)
    * @param index (Input) Which spectrometer should have its buffer cleared
    * @param error_code (Output) Pointer to allocated integer to receive error code
    */
    DLL_DECL void
    seabreeze_clear_buffer(int index, int *error_code);

    /**
    * @brief Get the number of spectra presently in the buffer (if equipped)
    * @param index (Input) Which spectrometer should have its buffer queried
    * @param error_code (Output) Pointer to allocated integer to receive error code
    * @return Number of spectra in the buffer
    */
    DLL_DECL unsigned long
    seabreeze_get_buffer_element_count(int index, int *error_code);

    /**
    * @brief Get the currently configured size of the data buffer (if equipped)
    * @param index (Input) Which spectrometer should have its buffer queried
    * @param error_code (Output) Pointer to allocated integer to receive error code
    * @return The present limit on the number of spectra that will be retained.
    */
    DLL_DECL unsigned long
    seabreeze_get_buffer_capacity(int index, int *error_code);

    /**
    * @brief Get the maximum possible configurable size for the data buffer (if equipped)
    * @param index (Input) Which spectrometer should have its buffer queried
    * @param error_code (Output) Pointer to allocated integer to receive error code
    * @return Maximum allowed value for the buffer size
    */
    DLL_DECL unsigned long
    seabreeze_get_buffer_capacity_maximum(int index, int *error_code);

    /**
    * @brief Get the minimum possible configurable size for the data buffer (if equipped)
    * @param index (Input) Which spectrometer should have its buffer queried
    * @param error_code (Output) Pointer to allocated integer to receive error code
    * @return Minimum allowed value for the buffer size
    */
    DLL_DECL unsigned long
    seabreeze_get_buffer_capacity_minimum(int index, int *error_code);

    /**
    * @brief Set the number of spectra that the buffer should keep
    * @param index (Input) Which spectrometer should have its buffer modified
    * @param error_code (Output) Pointer to allocated integer to receive error code
    * @param capacity (Input) Limit on number of spectra to store.
    *        Note that this must be within the range defined by the capacity minimum
    *        and maximum values.
    */
    DLL_DECL void
    seabreeze_set_buffer_capacity(int index, int *error_code, unsigned long capacity);

    /**
    * @brief Programmatically enable debug outputs to stderr
    * @param flag (Input) zero to disable (default), non-zero to enable
    */
    DLL_DECL void
    seabreeze_set_verbose(int flag);

    /**
    * @brief redirect verbose logging to named file
    * @param flag (Input) NULL for default behavior (stderr), non-null for valid OS path
    */
    DLL_DECL void
    seabreeze_set_logfile(char* pathname, int len);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* SEABREEZE_WRAPPER_H */
