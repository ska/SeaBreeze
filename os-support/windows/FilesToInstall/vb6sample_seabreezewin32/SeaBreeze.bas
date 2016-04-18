Attribute VB_Name = "SeaBreezeWin32"

'/**
' * This function opens a device attached to the system.
' * This can be called repeatedly with incrementing index values (until
' * it returns 1) to open all connected devices.
' *
' * Note that the index used to open a device with this function should also
' * be used to communicate with that same device in the other functions
' * provided here.
' *
' * PARAMETERS:
' *  index: The index of a USB device to try to open.  The valid values
' *         will range from 0 to N-1 for N connected devices.
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *
' * RETURN VALUES:
' *  int: The function will return an integer of 0 if it opened a
' *      device successfully, or 1 if no device was opened (in which
' *      case the error_code variable will be set).
Public Declare Function seabreeze_open_spectrometer Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long) As Long

'/**
'* This function closes the spectrometer attached to the system.
'*
'*
'* PARAMETERS:
'*  index: The index of a device previously opened with open_spectrometer().
'*  error_code: A pointer to an integer that can be used for storing
'*      error codes.
'*
'* RETURN VALUES:
'*  int: The function will return an integer of 0 if it closed the
'*      device successfully, or 1 if no device was closed (in which
'*      case the error_code variable will be set).
'*/
Public Declare Function seabreeze_close_spectrometer Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long) As Long

'/**
' * This function returns a description of the error denoted by
' * error_code.
' *
' *
' * PARAMETERS:
' *  error_code: The integer error code to look up.  Error codes may
' *      not be zero, but can be any non-zero integer (positive or
' *      negative).
' *
' * RETURN VALUES:
' *  char *: A description in the form of a string that describes
' *      what the error was.
' */
Public Declare Function seabreeze_get_error_string Lib "SeaBreezeWin32.dll" (ByVal error_code As Long, ByRef error_string As Byte, ByVal length As Long) As Long

'/**
' * This function returns a string denoting the type of the device.
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.  This may be NULL.
' *
' * RETURN VALUES:
' *  char *: A string that contains one of the following values:
' *      NONE: Used if no spectrometer is found (error_code will also be set)
' *      HR2000: Represents an HR2000 Spectrometer
' *      HR2000PLUS: Represents an HR2000+ Spectrometer
' *      HR4000: Represents an HR4000 Spectrometer
' *      MAYA2000: Represents a MAYA2000 Spectrometer
' *      MAYAPRO2000: Represents a MAYAPRO2000 Spectrometer
' *      NIRQUEST256: Represents an NIRQUEST256 Spectrometer
' *      NIRQUEST512: Represents an NIRQUEST512 Spectrometer
' *      QE65000: Represents a QE65000 Spectrometer
' *      USB2000: Represents a USB2000 Spectrometer
' *      USB2000PLUS: Represents a USB2000+ Spectrometer
' *      USB4000: Represents a USB4000 Spectrometer
' *
' * NOTE: This is not thread safe.  It returns a statically allocated
' *       character buffer that is reused on all subsequent calls.
' *       If you may be calling this from different threads, be sure
' *       to set a mutex around this call and copy the result to somewhere
' *       safe before releasing the mutex.  Do not attempt to free the
' *       returned buffer.
' */
Public Declare Function seabreeze_get_spectrometer_type Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long) As Long

'/**
' * This function sets the trigger mode for the specified device.
' * Note that requesting an unsupported mode will result in an error.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  mode: a trigger mode (0 = normal, 1 = software, 2 = synchronization,
' *        3 = external hardware, etc.)
' */
Public Declare Sub seabreeze_set_trigger_mode Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal mode As Long)

'/**
' * This function sets the integration time for the specified device.
' * This function should not be responsible for performing stability
' * scans.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  integration_time_micros: The new integration time in units of
' *      microseconds
' */
Public Declare Sub seabreeze_set_integration_time Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal integration_time_micros As Long)

'/**
' * This function returns the smallest integration time setting,
' * in microseconds, that is valid for the spectrometer.
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' * RETURN VALUE:
' *  Returns minimum legal integration time in microseconds if > 0.
' *  On error, returns -1 and error_code will be set accordingly.
' */
Public Declare Function seabreeze_get_minimum_integration_time_micros Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long) As Long

'/**
' * This function sets the strobe enable on the spectrometer.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  strobe_enable: A character used for denoting the desired value
' *      (high/low) of the strobe-enable pin.   If the value of
' *      strobe_enable is 0x00, then the pin should be set low.  If
' *      the value of strobe_enable is 0x01, then the pin should be
' *      set high.
' */
Public Declare Sub seabreeze_set_strobe_enable Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal strobe_enable As Integer)

'/**
' * This function reads a string out of the spectrometer's EEPROM slot
' * and returns the result.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  slot_number:  The number of the slot to read out.  Possible
' *      values are 0 through 17.
' *  buffer: A buffer (with memory already allocated) to hold the
' *      value read out of the EEPROM slot
' *  buffer_length: The length of the buffer
' *
' * RETURN VALUES:
' *  int: The number of bytes read from the EEPROM slot into the
' *      buffer
' */
Public Declare Function seabreeze_read_eeprom_slot Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal slot_number As Long, ByRef buffer As Double, ByVal buffer_length As Long) As Long

'/**
' * This function reads out an irradiance calibration from the spectrometer's
' * EEPROM if that feature is supported.
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *
' * RETURN VALUES:
' *  int: the number of floats read from the EEPROM into the buffer
' */
Public Declare Function seabreeze_read_irrad_calibration Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByRef buffer As Double, ByVal buffer_length As Long) As Long

'/**
' * This function reads the value of the TEC and returns the value in
' * degrees celsius.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *
' * RETURN VALUES:
' *  int: The TEC temperature in degrees celsius.
' */
Public Declare Function seabreeze_read_tec_temperature Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long) As Long

'/**
' * This function sets the TEC temperature.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  temperature_degrees_celsius: The desired temperature, in degrees
' *      celsius.
' */
Public Declare Sub seabreeze_set_tec_temperature Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal temperature_degrees_celsius As Long)

'/**
' * This function enables the TEC feature on the spectrometer.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  tec_enable: A character that denotes the desired TEC enable
' *      state.  If the value of tec_enable is 0x00, the TEC should
' *      be disabled.  If the value of tec_enable is 0x01, the TEC
' *      should be enabled.
' */
Public Declare Sub seabreeze_set_tec_enable Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal tec_enable As Integer)

'/**
' * This function enables the TEC Fan on the spectrometer.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  tec_enable: A character that denotes the desired TEC fan enable
' *      state.  If the value of tec_fan_enable is 0x00, the TEC fan
' *      should be disabled.  If the value of tec_fan_enable is 0x01,
' *      the TEC fan should be enabled.
' */
Public Declare Sub seabreeze_set_tec_fan_enable Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal tec_fan_enable As Integer)

'/**
' * This acquires a spectrum and returns the answer in raw,
' * unformatted bytes.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  buffer: A buffer (with memory already allocated) to hold the
' *      spectral data
' *  buffer_length: The length of the buffer
' *
' * RETURN VALUES:
' *  int: The number of bytes read into the buffer
' */
Public Declare Function seabreeze_get_unformatted_spectrum Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByRef buffer As Double, ByVal buffer_length As Long) As Long

'/**
' * This acquires a spectrum and returns the answer in formatted
' *     floats.  In this mode, auto-nulling should be automatically
' *     performed for devices that support it.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  buffer: A buffer (with memory already allocated) to hold the
' *      spectral data
' *  buffer_length: The length of the buffer
' *
' * RETURN VALUES:
' *  int: The number of floats read into the buffer
' */
Public Declare Function seabreeze_get_formatted_spectrum Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByRef buffer As Double, ByVal buffer_length As Long) As Long

'/**
' * This returns an integer denoting the length of a raw spectrum
' * (as returned by get_unformatted_spectrum(...)).
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *
' * RETURN VALUES:
' *  int: An integer denoting the length of an unformatted spectrum.
' */
Public Declare Function seabreeze_get_unformatted_spectrum_length Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long) As Long

'/**
' * This returns an integer denoting the number of pixels in a
' * formatted spectrum (as returned by get_formatted_spectrum(...)).
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *
' * RETURN VALUES:
' *  int: An integer denoting the length of a formatted spectrum.
' */
Public Declare Function seabreeze_get_formatted_spectrum_length Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long) As Long

'/**
' * This computes the wavelengths for the spectrometer and fills in the
' * provided array (up to the given length) with those values.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *
' *  wavelengths: A pre-allocated array of doubles that the wavelengths
' *               will be copied into
' *
' *  length: The number of values to copy into the wavelength array (this should
' *          be no larger than the number of doubles allocated in the wavelengths
' *          array)
' *
' * RETURN VALUES:
' *  int: An integer denoting the number of bytes written into the wavelength buffer
' */
Public Declare Function seabreeze_get_wavelengths Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByRef wavelengths As Double, ByVal length As Long) As Long

'/**
' * This fills in the provided array (up to the given length) with the indices
' * of the pixels that are electrically active but optically masked
' * (a.k.a. electric dark pixels).  Note that not all detectors have optically
' * masked pixels; in that case, this function will return zero.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *
' *  indices: A pre-allocated array of ints that the pixel indices
' *               will be copied into
' *
' *  length: The number of values to copy into the indices array (this should
' *          be no larger than the number of ints allocated in the indices
' *          array)
' *
' * RETURN VALUES:
' *  int: An integer denoting the number of bytes written into the indices buffer
' */
Public Declare Function seabreeze_get_electric_dark_pixel_indices Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByRef indices As Long, ByVal length As Long) As Long

'/**
' * This function reads two bytes out of the spectrometer's FPGA register
' * and returns the result.
' *
' *
' * PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  address:  The number of FPGA register to read out.
' *  buffer: A buffer (with memory already allocated) to hold the
' *      value read out of FPGA
' *  buffer_length: The length of the buffer
' *
' * RETURN VALUES:
' *  int: The number of bytes read from the FPGA into the
' *      buffer
' */
Public Declare Function seabreeze_read_FPGA Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal address As Long, ByRef buffer As Long, ByVal buffer_length As Long) As Long

'/**
' *  This function read the boolean value from certain GPIO pin
' *  and returns the result
' *
' *  PARAMETERS:
' *  index: The index of a device previously opened with open_spectrometer().
' *  error_code: A pointer to an integer that can be used for storing
' *      error codes.
' *  bit_number:  The number of the GPIO pin to read out.  Possible
' *      values are 0 through 8.
'
' * RETURN VALUES:
' *  int: output value of this GPIO pin. Possible value is 0 or 1
' */
Public Declare Function seabreeze_read_GPIOBit Lib "SeaBreezeWin32.dll" (ByVal index As Long, ByRef error_code As Long, ByVal bit_number As Long) As Long
