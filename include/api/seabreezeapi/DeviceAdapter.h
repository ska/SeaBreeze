/***************************************************//**
 * @file    DeviceAdapter.h
 * @date    January 2015
 * @author  Ocean Optics, Inc., Kirk Clendinning, Heliospectra
 *
 * This is a wrapper that allows
 * access to SeaBreeze Device instances.
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
#ifndef DEVICEADAPTER_H
#define DEVICEADAPTER_H

/* Includes */
#include "common/devices/Device.h"
#include "common/buses/DeviceLocatorInterface.h"
#include "api/seabreezeapi/EEPROMFeatureAdapter.h"
#include "api/seabreezeapi/IrradCalFeatureAdapter.h"
#include "api/seabreezeapi/RawUSBBusAccessFeatureAdapter.h"
#include "api/seabreezeapi/NonlinearityCoeffsFeatureAdapter.h"
#include "api/seabreezeapi/TemperatureFeatureAdapter.h"
#include "api/seabreezeapi/RevisionFeatureAdapter.h"
#include "api/seabreezeapi/OpticalBenchFeatureAdapter.h"
#include "api/seabreezeapi/SpectrumProcessingFeatureAdapter.h"
#include "api/seabreezeapi/SerialNumberFeatureAdapter.h"
#include "api/seabreezeapi/ShutterFeatureAdapter.h"
#include "api/seabreezeapi/SpectrometerFeatureAdapter.h"
#include "api/seabreezeapi/StrayLightCoeffsFeatureAdapter.h"
#include "api/seabreezeapi/StrobeLampFeatureAdapter.h"
#include "api/seabreezeapi/ContinuousStrobeFeatureAdapter.h"
#include "api/seabreezeapi/ThermoElectricCoolerFeatureAdapter.h"
#include "api/seabreezeapi/LightSourceFeatureAdapter.h"
#include "api/seabreezeapi/PixelBinningFeatureAdapter.h"
#include "api/seabreezeapi/DataBufferFeatureAdapter.h"
#include "api/seabreezeapi/AcquisitionDelayFeatureAdapter.h"
#include <vector>

namespace seabreeze {
    namespace api {

        class DeviceAdapter {
        public:
            DeviceAdapter(Device *dev, unsigned long id);
            ~DeviceAdapter();

            int open(int *errorCode);
            void close();

            DeviceLocatorInterface *getLocation();

            /* An for weak association to this object */
            unsigned long getID();

            /* Get a string that describes the type of device */
            int getDeviceType(int *errorCode, char *buffer, unsigned int maxLength);
            
            /* Get a usb endpoint for the device according to the enumerator */
            /*  endpointType. A 0 is returned if the endpoint requested is not in use. */
            unsigned char getDeviceEndpoint(int *errorCode, usbEndpointType anEndpointType);

            /* Get one or more raw USB access features */
            int getNumberOfRawUSBBusAccessFeatures();
            int getRawUSBBusAccessFeatures(long *buffer, int maxFeatures);
            int rawUSBBusAccessRead(long featureID,
                    int *errorCode, unsigned char *buffer, unsigned int bufferLength, unsigned char endpoint);
            int rawUSBBusAccessWrite(long featureID,
                    int *errorCode, unsigned char *buffer, unsigned int bufferLength, unsigned char endpoint);
                    
            /* Get one or more serial number features */
            int getNumberOfSerialNumberFeatures();
            int getSerialNumberFeatures(long *buffer, int maxFeatures);
            int getSerialNumber(long featureID, int *errorCode,
                    char *buffer, int bufferLength);
            unsigned char getSerialNumberMaximumLength(long featureID, int *errorCode);

            /* Get one or more spectrometer acquisition features */
            int getNumberOfSpectrometerFeatures();
            int getSpectrometerFeatures(long *buffer, int maxFeatures);
            void spectrometerSetTriggerMode(long spectrometerFeatureID, int *errorCode, int mode);
            void spectrometerSetIntegrationTimeMicros(long spectrometerFeatureID, int *errorCode,
                    unsigned long integrationTimeMicros);
            unsigned long spectrometerGetMinimumIntegrationTimeMicros(
                    long spectrometerFeatureID, int *errorCode);
            unsigned long spectrometerGetMaximumIntegrationTimeMicros(
                    long spectrometerFeatureID, int *errorCode);
            double spectrometerGetMaximumIntensity(
                    long spectrometerFeatureID, int *errorCode);
            int spectrometerGetUnformattedSpectrumLength(
                    long spectrometerFeatureID, int *errorCode);
            int spectrometerGetUnformattedSpectrum(long spectrometerFeatureID,
                    int *errorCode, unsigned char *buffer, int bufferLength);
            int spectrometerGetFormattedSpectrumLength(
                    long spectrometerFeatureID, int *errorCode);
            int spectrometerGetFormattedSpectrum(long spectrometerFeatureID, int *errorCode,
                    double *buffer, int bufferLength);
            int spectrometerGetWavelengths(long spectrometerFeatureID, int *errorCode,
                    double *wavelengths, int length);
            int spectrometerGetElectricDarkPixelCount(
                    long spectrometerFeatureID, int *errorCode);
            int spectrometerGetElectricDarkPixelIndices(
                    long spectrometerFeatureID, int *errorCode, int *indices, int length);


            /* Get one or more pixel binning features */
            int getNumberOfPixelBinningFeatures();
            int getPixelBinningFeatures(long *buffer, int maxFeatures);
            void binningSetPixelBinningFactor(long spectrometerFeatureID, int *errorCode, const unsigned char binningFactor);
            unsigned char binningGetPixelBinningFactor(long spectrometerFeatureID, int *errorCode);
            void binningSetDefaultPixelBinningFactor(long spectrometerFeatureID, int *errorCode, const unsigned char binningFactor);
            void binningSetDefaultPixelBinningFactor(long spectrometerFeatureID, int *errorCode);
            unsigned char binningGetDefaultPixelBinningFactor(long spectrometerFeatureID, int *errorCode);
            unsigned char binningGetMaxPixelBinningFactor(long spectrometerFeatureID, int *errorCode);
            
            /* Get one or more TEC features */
            int getNumberOfThermoElectricFeatures();
            int getThermoElectricFeatures(long *buffer, int maxFeatures);
            double tecReadTemperatureDegreesC(long featureID, int *errorCode);
            void tecSetTemperatureSetpointDegreesC(long featureID, int *errorCode,
                    double temperatureDegreesCelsius);
            void tecSetEnable(long featureID, int *errorCode, bool tecEnable);

            /* Get one or more irradiance calibration features */
            int getNumberOfIrradCalFeatures();
            int getIrradCalFeatures(long *buffer, int maxFeatures);
            int irradCalibrationRead(long featureID,
                    int *errorCode, float *buffer, int bufferLength);
            int irradCalibrationWrite(long featureID,
                    int *errorCode, float *buffer, int bufferLength);
            int irradCalibrationHasCollectionArea(long featureID, int *errorCode);
            float irradCalibrationReadCollectionArea(long featureID, int *errorCode);
            void irradCalibrationWriteCollectionArea(long featureID,
                    int *errorCode, float area);

            /* Get one or more EEPROM features */
            int getNumberOfEEPROMFeatures();
            int getEEPROMFeatures(long *buffer, int maxFeatures);
            int eepromReadSlot(long featureID, int *errorCode, int slotNumber,
                    unsigned char *buffer, int length);

            /* Get one or more light source features */
            int getNumberOfLightSourceFeatures();
            int getLightSourceFeatures(long *buffer, int maxFeatures);
            int lightSourceGetCount(long featureID, int *errorCode);
            bool lightSourceHasEnable(long featureID, int *errorCode,
                int lightSourceIndex);
            bool lightSourceIsEnabled(long featureID, int *errorCode,
                int lightSourceIndex);
            void lightSourceSetEnable(long featureID, int *errorCode,
                int lightSourceIndex, bool enable);
            bool lightSourceHasVariableIntensity(long featureID, int *errorCode,
                int lightSourceIndex);
            double lightSourceGetIntensity(long featureID, int *errorCode,
                int lightSourceIndex);
            void lightSourceSetIntensity(long featureID, int *errorCode,
                int lightSourceIndex, double intensity);

            /* Get one or more strobe lamp enable features */
            int getNumberOfStrobeLampFeatures();
            int getStrobeLampFeatures(long *buffer, int maxFeatures);
            void lampSetStrobeEnable(long featureID, int *errorCode, bool strobeEnable);

            /* Get one or more continuous strobe features */
            int getNumberOfContinuousStrobeFeatures();
            int getContinuousStrobeFeatures(long *buffer, int maxFeatures);
            void continuousStrobeSetPeriodMicroseconds(long featureID, int *errorCode,
                unsigned long period_usec);
            void continuousStrobeSetEnable(long featureID, int *errorCode, bool enable);

            /* Get one or more shutter features */
            int getNumberOfShutterFeatures();
            int getShutterFeatures(long *buffer, int maxFeatures);
            void shutterSetShutterOpen(long featureID, int *errorCode, bool opened);

            /* Get one or more nonlinearity coefficients features */
            int getNumberOfNonlinearityCoeffsFeatures();
            int getNonlinearityCoeffsFeatures(long *buffer, int maxFeatures);
            int nonlinearityCoeffsGet(long featureID, int *errorCode,
                    double *buffer, int bufferLength);
                    
            /* Get one or more temperature features */
            int getNumberOfTemperatureFeatures();
            int getTemperatureFeatures(long *buffer, int maxFeatures);
            unsigned char temperatureCountGet(long temperatureFeatureID, int *errorCode);
            double temperatureGet(long temperatureFeatureID, int *errorCode, int index);
            int temperatureGetAll(long temperatureFeatureID, int *errorCode,
                    double *buffer, int bufferLength);

            /* Get one or more revision features */
            int getNumberOfRevisionFeatures();
            int getRevisionFeatures(long *buffer, int maxFeatures);
            unsigned char revisionHardwareGet(long revisionFeatureID, int *errorCode);
            unsigned short int revisionFirmwareGet(long revisionFeatureID, int *errorCode);

            /* Get one or more spectrum processing features */
            int getNumberOfSpectrumProcessingFeatures();
            int getSpectrumProcessingFeatures(long *buffer, int maxFeatures);
            unsigned short int spectrumProcessingScansToAverageGet(long spectrumProcessingFeatureID, int *errorCode);
            unsigned char spectrumProcessingBoxcarWidthGet(long spectrumProcessingFeatureID, int *errorCode);
            void spectrumProcessingBoxcarWidthSet(long featureID, int *errorCode, unsigned char boxcarWidth);
            void spectrumProcessingScansToAverageSet(long featureID, int *errorCode, unsigned short int scansToAverage);
                                                   
            /* Get one or more optical bench features */
            int getNumberOfOpticalBenchFeatures();
            int getOpticalBenchFeatures(long *buffer, int maxFeatures);
            unsigned short int opticalBenchGetFiberDiameterMicrons(long opticalBenchFeatureID, int *errorCode);
            unsigned short int opticalBenchGetSlitWidthMicrons(long opticalBenchFeatureID, int *errorCode);
            int opticalBenchGetID(long opticalBenchFeatureID, int *errorCode, char *buffer, int bufferLength);
            int opticalBenchGetSerialNumber(long opticalBenchFeatureID, int *errorCode, char *buffer, int bufferLength);
            int opticalBenchGetCoating(long opticalBenchFeatureID, int *errorCode, char *buffer, int bufferLength);
            int opticalBenchGetFilter(long opticalBenchFeatureID, int *errorCode, char *buffer, int bufferLength);
            int opticalBenchGetGrating(long opticalBenchFeatureID, int *errorCode, char *buffer, int bufferLength);

            
            /* Get one or more stray light coefficients features */
            int getNumberOfStrayLightCoeffsFeatures();
            int getStrayLightCoeffsFeatures(long *buffer, int maxFeatures);
            int strayLightCoeffsGet(long featureID, int *errorCode,
                    double *buffer, int bufferLength);

            /* Get one or more data buffer features */
            int getNumberOfDataBufferFeatures();
            int getDataBufferFeatures(long *buffer, int maxFeatures);
            void dataBufferClear(long featureID, int *errorCode);
            unsigned long dataBufferGetNumberOfElements(long featureID, int *errorCode);
            unsigned long dataBufferGetBufferCapacity(long featureID, int *errorCode);
            unsigned long dataBufferGetBufferCapacityMaximum(long featureID, int *errorCode);
            unsigned long dataBufferGetBufferCapacityMinimum(long featureID, int *errorCode);
            void dataBufferSetBufferCapacity(long featureID, int *errorCode, unsigned long capacity);

            /* Get one or more acquisition delay features */
            int getNumberOfAcquisitionDelayFeatures();
            int getAcquisitionDelayFeatures(long *buffer, int maxFeatures);
            void acquisitionDelaySetDelayMicroseconds(long featureID, int *errorCode,
                unsigned long delay_usec);
            unsigned long acquisitionDelayGetDelayMicroseconds(long featureID, int *errorCode);
            unsigned long acquisitionDelayGetDelayIncrementMicroseconds(long featureID, int *errorCode);
            unsigned long acquisitionDelayGetDelayMaximumMicroseconds(long featureID, int *errorCode);
            unsigned long acquisitionDelayGetDelayMinimumMicroseconds(long featureID, int *errorCode);

        protected:
            unsigned long instanceID;
            seabreeze::Device *device;
            std::vector<RawUSBBusAccessFeatureAdapter *> rawUSBBusAccessFeatures;
            std::vector<SerialNumberFeatureAdapter *> serialNumberFeatures;
            std::vector<SpectrometerFeatureAdapter *> spectrometerFeatures;
            std::vector<ThermoElectricCoolerFeatureAdapter *> tecFeatures;
            std::vector<IrradCalFeatureAdapter *> irradCalFeatures;
            std::vector<EEPROMFeatureAdapter *> eepromFeatures;
            std::vector<LightSourceFeatureAdapter *> lightSourceFeatures;
            std::vector<StrobeLampFeatureAdapter *> strobeLampFeatures;
            std::vector<ContinuousStrobeFeatureAdapter *> continuousStrobeFeatures;
            std::vector<ShutterFeatureAdapter *> shutterFeatures;
            std::vector<NonlinearityCoeffsFeatureAdapter *> nonlinearityFeatures;
            std::vector<TemperatureFeatureAdapter *> temperatureFeatures;
            std::vector<RevisionFeatureAdapter *> revisionFeatures;
            std::vector<OpticalBenchFeatureAdapter *> opticalBenchFeatures;
            std::vector<SpectrumProcessingFeatureAdapter *> spectrumProcessingFeatures;
            std::vector<StrayLightCoeffsFeatureAdapter *> strayLightFeatures;
            std::vector<PixelBinningFeatureAdapter *> pixelBinningFeatures;
            std::vector<DataBufferFeatureAdapter *> dataBufferFeatures;
            std::vector<AcquisitionDelayFeatureAdapter *> acquisitionDelayFeatures;
            
            RawUSBBusAccessFeatureAdapter *getRawUSBBusAccessFeatureByID(long featureID);
            SerialNumberFeatureAdapter *getSerialNumberFeatureByID(long featureID);
            SpectrometerFeatureAdapter *getSpectrometerFeatureByID(long featureID);
            ThermoElectricCoolerFeatureAdapter *getTECFeatureByID(long featureID);
            IrradCalFeatureAdapter *getIrradCalFeatureByID(long featureID);
            EEPROMFeatureAdapter *getEEPROMFeatureByID(long featureID);
            LightSourceFeatureAdapter *getLightSourceFeatureByID(long featureID);
            StrobeLampFeatureAdapter *getStrobeLampFeatureByID(long featureID);
            ContinuousStrobeFeatureAdapter *getContinuousStrobeFeatureByID(long featureID);
            ShutterFeatureAdapter *getShutterFeatureByID(long featureID);
            NonlinearityCoeffsFeatureAdapter *getNonlinearityCoeffsFeatureByID(long featureID);
            TemperatureFeatureAdapter *getTemperatureFeatureByID(long featureID);
            RevisionFeatureAdapter *getRevisionFeatureByID(long featureID);
            OpticalBenchFeatureAdapter *getOpticalBenchFeatureByID(long featureID);
            SpectrumProcessingFeatureAdapter *getSpectrumProcessingFeatureByID(long featureID);
            StrayLightCoeffsFeatureAdapter *getStrayLightCoeffsFeatureByID(long featureID);
            PixelBinningFeatureAdapter *getPixelBinningFeatureByID(long featureID);
            DataBufferFeatureAdapter *getDataBufferFeatureByID(long featureID);
            AcquisitionDelayFeatureAdapter *getAcquisitionDelayFeatureByID(long featureID);
        };
    }
}

#endif

