// This file is not yet ready for use, but is provided as a starting-point for when we need it.

// Initialization
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_initialize();
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_shutdown();
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_probe_devices();
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_device_ids();
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_device_ids                                       (ref long ids, unsigned int max_ids);
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_open_device                                          (long id, ref int error_code);
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_close_device                                         (long id, ref int error_code);

// Spectrometer Features
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_device_type                                      (long id, ref int error_code, ref byte buffer, unsigned int length);
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_serial_number_features                 (long deviceID, ref int error_code);
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_serial_number_features                           (long deviceID, ref int error_code, ref long features, int max_features);
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_serial_number                                    (long deviceID, long featureID, ref int error_code, ref byte buffer, int buffer_length); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_spectrometer_features                  (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_spectrometer_features                            (long deviceID, ref int error_code, ref long features, int max_features);
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_spectrometer_set_trigger_mode                        (long deviceID, long featureID, ref int error_code, int mode); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_spectrometer_set_integration_time_micros             (long deviceID, long featureID, ref int error_code, unsigned long integration_time_micros);
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern long   sbapi_spectrometer_get_minimum_integration_time_micros     (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_spectrometer_get_formatted_spectrum_length           (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_spectrometer_get_formatted_spectrum                  (long deviceID, long featureID, ref int error_code, ref double buffer, int buffer_length); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_spectrometer_get_unformatted_spectrum_length         (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_spectrometer_get_unformatted_spectrum                (long deviceID, long featureID, ref int error_code, ref byte buffer, int buffer_length); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_spectrometer_get_wavelengths                         (long deviceID, long featureID, ref int error_code, ref double wavelengths, int length); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_spectrometer_get_electric_dark_pixel_count           (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_spectrometer_get_electric_dark_pixel_indices         (long deviceID, long featureID, ref int error_code, ref int indices, int length); 

// EEPROM
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_eeprom_features                        (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_eeprom_features                                  (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_eeprom_read_slot                                     (long deviceID, long featureID, ref int error_code, int slot_number, ref byte buffer, int buffer_length); 

// NLC
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_nonlinearity_coeffs_features           (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_nonlinearity_coeffs_features                     (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_nonlinearity_coeffs_get                              (long deviceID, long featureID, ref int error_code, ref double buffer, int max_length); 

// Shutter
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_shutter_features                       (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_shutter_features                                 (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_shutter_set_shutter_open                             (long deviceID, long featureID, ref int error_code, byte opened); 

// Light Sources
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_light_source_features                  (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_light_source_features                            (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_light_source_get_count                               (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern byte   sbapi_light_source_has_enable                              (long deviceID, long featureID, ref int error_code, int light_source_index); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern byte   sbapi_light_source_is_enabled                              (long deviceID, long featureID, ref int error_code, int light_source_index); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_light_source_set_enable                              (long deviceID, long featureID, ref int error_code, int light_source_index, byte enable); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern byte   sbapi_light_source_has_variable_intensity                  (long deviceID, long featureID, ref int error_code, int light_source_index); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern double sbapi_light_source_get_intensity                           (long deviceID, long featureID, ref int error_code, int light_source_index); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_light_source_set_intensity                           (long deviceID, long featureID, ref int error_code, int light_source_index, double intensity); 

// Strobe
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_strobe_lamp_features                   (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_strobe_lamp_features                             (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_lamp_features                          (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_lamp_features                                    (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_lamp_set_lamp_enable                                 (long deviceID, long featureID, ref int error_code, byte lamp_enable); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_continuous_strobe_features             (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_continuous_strobe_features                       (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_continuous_strobe_set_continuous_strobe_enable       (long deviceID, long featureID, ref int error_code, byte enable); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_continuous_strobe_set_continuous_strobe_period_micros(long deviceID, long featureID, ref int error_code, unsigned long period_micros); 

// Irradiance 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_irrad_cal_features                     (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_irrad_cal_features                               (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_irrad_calibration_read                               (long deviceID, long featureID, ref int error_code, ref float buffer, int buffer_length); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_irrad_calibration_write                              (long deviceID, long featureID, ref int error_code, ref float buffer, int buffer_length); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_irrad_calibration_has_collection_area                (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern float  sbapi_irrad_calibration_read_collection_area               (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_irrad_calibration_write_collection_area              (long deviceID, long featureID, ref int error_code, float area); 

// TEC
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_thermo_electric_features               (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_thermo_electric_features                         (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern double sbapi_tec_read_temperature_degrees_C                       (long deviceID, long featureID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_tec_set_temperature_setpoint_degrees_C               (long deviceID, long featureID, ref int error_code, double temperature_degrees_celsius); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern void   sbapi_tec_set_enable                                       (long deviceID, long featureID, ref int error_code, byte tec_enable); 

// Stray Light
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_number_of_stray_light_coeffs_features            (long deviceID, ref int error_code); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_get_stray_light_coeffs_features                      (long deviceID, ref int error_code, ref long features, int max_features); 
[DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int    sbapi_stray_light_coeffs_get                               (long deviceID, long featureID, ref int error_code, ref double buffer, int max_length);
