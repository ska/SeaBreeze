/***************************************************//**
 * @file    DeviceAdapter.cpp
 * @date    January 2015
 * @author  Ocean Optics, Inc., Kirk Clendinning, Heliospectra
 *
 * This is a wrapper that allows
 * access to SeaBreeze DeviceInterface instances.
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

#include "common/globals.h"
#include "api/seabreezeapi/DeviceAdapter.h"  // references device.h
#include "api/seabreezeapi/FeatureFamilies.h"
#include "api/seabreezeapi/SeaBreezeAPIConstants.h"
#include <string>
#include <string.h>

using namespace seabreeze;
using namespace seabreeze::api;
using namespace std;

template <class T>
vector<T *> *__sbapi_getFeatures(Device *dev) {
    /* This is a templated function to get all of the features of a particular
     * type based on the feature interface.  These can be used directly to
     * create feature adapters, but the logic to do that is too much for a
     * simple templated function to deal with.
     */
    vector<Feature *>::iterator iter;
    vector<Feature *> features = dev->getFeatures();
    vector<T *> *retval = new vector<T *>();
    for(iter = features.begin(); iter != features.end(); iter++) {
        T *intf = dynamic_cast<T *>(*iter);
        if(NULL != intf) {
            retval->push_back(intf);
        }
    }
    return retval;
}

DeviceAdapter::DeviceAdapter(Device *dev, unsigned long id) {
    this->device = dev;
    this->instanceID = id;

    if(NULL == this->device) {
        std::string error("Null device is not allowed.");
        throw IllegalArgumentException(error);
    }
}

template <class T> void __delete_feature_adapters(vector<T *> &features) {
    unsigned int i;

    for(i = 0; i < features.size(); i++) {
        delete features[i];
    }
}

DeviceAdapter::~DeviceAdapter() {
    __delete_feature_adapters<RawUSBBusAccessFeatureAdapter>(rawUSBBusAccessFeatures);
    __delete_feature_adapters<SerialNumberFeatureAdapter>(serialNumberFeatures);
    __delete_feature_adapters<SpectrometerFeatureAdapter>(spectrometerFeatures);
    __delete_feature_adapters<ThermoElectricCoolerFeatureAdapter>(tecFeatures);
    __delete_feature_adapters<IrradCalFeatureAdapter>(irradCalFeatures);
    __delete_feature_adapters<EEPROMFeatureAdapter>(eepromFeatures);
    __delete_feature_adapters<StrobeLampFeatureAdapter>(strobeLampFeatures);
    __delete_feature_adapters<ContinuousStrobeFeatureAdapter>(continuousStrobeFeatures);
    __delete_feature_adapters<ShutterFeatureAdapter>(shutterFeatures);
    __delete_feature_adapters<NonlinearityCoeffsFeatureAdapter>(nonlinearityFeatures);
    __delete_feature_adapters<TemperatureFeatureAdapter>(temperatureFeatures);
    __delete_feature_adapters<RevisionFeatureAdapter>(revisionFeatures);
    __delete_feature_adapters<OpticalBenchFeatureAdapter>(opticalBenchFeatures);
    __delete_feature_adapters<SpectrumProcessingFeatureAdapter>(spectrumProcessingFeatures);
    __delete_feature_adapters<StrayLightCoeffsFeatureAdapter>(strayLightFeatures);
    __delete_feature_adapters<LightSourceFeatureAdapter>(lightSourceFeatures);
    __delete_feature_adapters<PixelBinningFeatureAdapter>(pixelBinningFeatures);
    __delete_feature_adapters<DataBufferFeatureAdapter>(dataBufferFeatures);
    __delete_feature_adapters<AcquisitionDelayFeatureAdapter>(acquisitionDelayFeatures);

    delete this->device;
}

template <class T, class U> void __create_feature_adapters(Device *device,
        vector<U *> &adapters, Bus *bus, const FeatureFamily &family) {

    unsigned short i;
    vector<Protocol *> protocols;

    /* Purge anything already in the adapters */
    adapters.clear();

    /* Create feature list */
    vector<T *> *features = __sbapi_getFeatures<T>(device);
    for(i = 0; i < features->size(); i++) {
        /* All FeatureInterface instances should also be Features, but
         * do a proper cast to be sure.
         */
        Feature *f = dynamic_cast<Feature *>((*features)[i]);
        if(NULL == f) {
            /* This should never happen if the class is defined properly. */
            continue;
        }
        ProtocolFamily protocolFamily = device->getSupportedProtocol(
                f->getFeatureFamily(), bus->getBusFamily());
        protocols = device->getProtocolsByFamily(protocolFamily);
        if(protocols.size() < 1) {
            /* No supported protocol for this feature on the opened bus. */
            continue;
        }
        adapters.push_back(
            new U(((*features)[i]), family, (protocols[0]), bus, i));
    }
    delete features;
}


int DeviceAdapter::open(int *errorCode) {
    Bus *bus;
    vector<Protocol *> protocols;
    FeatureFamilies featureFamilies;
    int flag;

    flag = this->device->open();
    if(0 != flag || NULL == this->device->getOpenedBus()) {
        /* Failed to open the device. */
        SET_ERROR_CODE(ERROR_NO_DEVICE);
        return flag;
    }

    bus = this->device->getOpenedBus();

    /* This gives the device a chance to probe the hardware and update its
     * set of Feature instances based on what is detected.
     */
    this->device->initialize(*bus);
    
    /* Create raw usb access feature list */
    __create_feature_adapters<RawUSBBusAccessFeatureInterface,
    	            RawUSBBusAccessFeatureAdapter>(this->device,
            rawUSBBusAccessFeatures, bus, featureFamilies.RAW_USB_BUS_ACCESS);

    /* Create serial number feature list */
    __create_feature_adapters<SerialNumberFeatureInterface,
                    SerialNumberFeatureAdapter>(this->device,
            serialNumberFeatures, bus, featureFamilies.SERIAL_NUMBER);

    /* Create spectrometer feature list */
    __create_feature_adapters<OOISpectrometerFeatureInterface,
                    SpectrometerFeatureAdapter>(this->device,
            spectrometerFeatures, bus, featureFamilies.SPECTROMETER);

    /* Create TEC feature list */
    __create_feature_adapters<ThermoElectricFeatureInterface,
                    ThermoElectricCoolerFeatureAdapter>(this->device,
            tecFeatures, bus, featureFamilies.THERMOELECTRIC);

    /* Create irradiance calibration feature list */
    __create_feature_adapters<IrradCalFeatureInterface,
                    IrradCalFeatureAdapter>(this->device,
            irradCalFeatures, bus, featureFamilies.IRRAD_CAL);

    /* Create EEPROM feature list */
    __create_feature_adapters<EEPROMSlotFeatureInterface,
                    EEPROMFeatureAdapter>(this->device,
            eepromFeatures, bus, featureFamilies.EEPROM);

    /* Create light source feature list */
    __create_feature_adapters<LightSourceFeatureInterface,
                    LightSourceFeatureAdapter>(this->device,
            lightSourceFeatures, bus, featureFamilies.LIGHT_SOURCE);

    /* Create strobe/lamp enable feature list */
    __create_feature_adapters<StrobeLampFeatureInterface,
                    StrobeLampFeatureAdapter>(this->device,
            strobeLampFeatures, bus, featureFamilies.STROBE_LAMP_ENABLE);

    /* Create continuous strobe feature list */
    __create_feature_adapters<ContinuousStrobeFeatureInterface,
                    ContinuousStrobeFeatureAdapter>(this->device,
            continuousStrobeFeatures, bus, featureFamilies.CONTINUOUS_STROBE);

    /* Create shutter feature list */
    __create_feature_adapters<ShutterFeatureInterface,
                    ShutterFeatureAdapter>(this->device,
            shutterFeatures, bus, featureFamilies.SHUTTER);

    /* Create nonlinearity coefficients feature list */
    __create_feature_adapters<NonlinearityCoeffsFeatureInterface,
                    NonlinearityCoeffsFeatureAdapter>(this->device,
            nonlinearityFeatures, bus, featureFamilies.NONLINEARITY_COEFFS);

    /* Create temperature feature list */
    __create_feature_adapters<TemperatureFeatureInterface,
                    TemperatureFeatureAdapter>(this->device,
            temperatureFeatures, bus, featureFamilies.TEMPERATURE);

    /* Create revision feature list */
    __create_feature_adapters<RevisionFeatureInterface,
                    RevisionFeatureAdapter>(this->device,
            revisionFeatures, bus, featureFamilies.REVISION);
             
     /* Create optical bench feature list */
    __create_feature_adapters<OpticalBenchFeatureInterface,
                    OpticalBenchFeatureAdapter>(this->device,
            opticalBenchFeatures, bus, featureFamilies.OPTICAL_BENCH);
            
     /* Create spectrum processing feature list */
    __create_feature_adapters<SpectrumProcessingFeatureInterface,
                    SpectrumProcessingFeatureAdapter>(this->device,
            spectrumProcessingFeatures, bus, featureFamilies.SPECTRUM_PROCESSING);
                       
    /* Create stray light coefficients feature list */
    __create_feature_adapters<StrayLightCoeffsFeatureInterface,
                    StrayLightCoeffsFeatureAdapter>(this->device,
            strayLightFeatures, bus, featureFamilies.STRAY_LIGHT_COEFFS);

    /* Create pixel binning feature list */
    __create_feature_adapters<PixelBinningFeatureInterface,
                    PixelBinningFeatureAdapter>(this->device,
            pixelBinningFeatures, bus, featureFamilies.PIXEL_BINNING);

    __create_feature_adapters<DataBufferFeatureInterface,
                    DataBufferFeatureAdapter>(this->device,
            dataBufferFeatures, bus, featureFamilies.DATA_BUFFER);

    __create_feature_adapters<AcquisitionDelayFeatureInterface,
                    AcquisitionDelayFeatureAdapter>(this->device,
            acquisitionDelayFeatures, bus, featureFamilies.ACQUISITION_DELAY);

    SET_ERROR_CODE(ERROR_SUCCESS);
    return 0;
}

void DeviceAdapter::close() {
    this->device->close();
}

DeviceLocatorInterface *DeviceAdapter::getLocation() {
    return this->device->getLocation();
}

unsigned long DeviceAdapter::getID() {
    return this->instanceID;
}

int DeviceAdapter::getDeviceType(int *errorCode, char *buffer, unsigned int maxLength) {
    if(0 == maxLength || NULL == buffer) {
        SET_ERROR_CODE(ERROR_BAD_USER_BUFFER);
        /* Nothing to do */
        return 0;
    }

    string name = this->device->getName();
    string::iterator iter;

    memset(buffer, (int)0, maxLength);

    unsigned int i = 0;
    for(iter = name.begin(); iter != name.end() && i < maxLength; iter++) {
        if('+' == *iter) {
            strcat(buffer, "PLUS");
            i += 4;
        } else {
            buffer[i] = toupper(*iter);
            i++;
        }
    }

    buffer[maxLength - 1] = '\0'; /* Guarantee a null terminator */

    SET_ERROR_CODE(ERROR_SUCCESS);
    return i;
}


template <class T> int __getFeatureIDs(vector<T *> features, long *buffer, unsigned int max) {
    unsigned int i;

    for(i = 0; i < max && i < features.size(); i++) {
        buffer[i] = features[i]->getID();
    }

    return i;
}

template <class T> T *__getFeatureByID(vector<T *> features, long id) {
    unsigned int i;

    for(i = 0; i < features.size(); i++) {
        if(features[i]->getID() == id) {
            return features[i];
        }
    }

    return NULL;
}


/* returns a USB endpoint from the device as specified by the enumerator endpointType */
/*  if the endpoint type is not supported, a 0 is returned. */
unsigned char DeviceAdapter::getDeviceEndpoint(int *errorCode, usbEndpointType endpointType) 
{
    return this->device->getEndpoint(errorCode, endpointType);
}

/* Raw USB Access  feature wrappers */
int DeviceAdapter::getNumberOfRawUSBBusAccessFeatures() {
    return (int) this->rawUSBBusAccessFeatures.size();
}

int DeviceAdapter::getRawUSBBusAccessFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<RawUSBBusAccessFeatureAdapter>(
                rawUSBBusAccessFeatures, buffer, maxFeatures);
}

RawUSBBusAccessFeatureAdapter *DeviceAdapter::getRawUSBBusAccessFeatureByID(long featureID) {
    return __getFeatureByID<RawUSBBusAccessFeatureAdapter>(
                rawUSBBusAccessFeatures, featureID);
}


int DeviceAdapter::rawUSBBusAccessRead(long featureID,
        int *errorCode, unsigned char *buffer, unsigned int bufferLength, unsigned char endpoint) {
    RawUSBBusAccessFeatureAdapter *feature = getRawUSBBusAccessFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readUSB(errorCode, buffer, bufferLength, endpoint);
}

int DeviceAdapter::rawUSBBusAccessWrite(long featureID,
        int *errorCode, unsigned char *buffer, unsigned int bufferLength, unsigned char endpoint) {
    RawUSBBusAccessFeatureAdapter *feature = getRawUSBBusAccessFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->writeUSB(errorCode, buffer, bufferLength, endpoint);
}


/* Serial number feature wrappers */
int DeviceAdapter::getNumberOfSerialNumberFeatures() {
    return (int) this->serialNumberFeatures.size();
}

int DeviceAdapter::getSerialNumberFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<SerialNumberFeatureAdapter>(
                serialNumberFeatures, buffer, maxFeatures);
}

SerialNumberFeatureAdapter *DeviceAdapter::getSerialNumberFeatureByID(long featureID) {
    return __getFeatureByID<SerialNumberFeatureAdapter>(
                serialNumberFeatures, featureID);
}

int DeviceAdapter::getSerialNumber(long featureID, int *errorCode,
        char *buffer, int bufferLength) {
    SerialNumberFeatureAdapter *feature = getSerialNumberFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getSerialNumber(errorCode, buffer, bufferLength);
}

unsigned char DeviceAdapter::getSerialNumberMaximumLength(long featureID, int *errorCode) {
    SerialNumberFeatureAdapter *feature = getSerialNumberFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getSerialNumberMaximumLength(errorCode);
}

/* Spectrometer feature wrappers */
int DeviceAdapter::getNumberOfSpectrometerFeatures() {
    return (int) this->spectrometerFeatures.size();
}

int DeviceAdapter::getSpectrometerFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<SpectrometerFeatureAdapter>(
                spectrometerFeatures, buffer, maxFeatures);
}

SpectrometerFeatureAdapter *DeviceAdapter::getSpectrometerFeatureByID(long featureID) {
    return __getFeatureByID<SpectrometerFeatureAdapter>(
                spectrometerFeatures, featureID);
}

void DeviceAdapter::spectrometerSetTriggerMode(long featureID,
            int *errorCode, int mode) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setTriggerMode(errorCode, mode);
}

void DeviceAdapter::spectrometerSetIntegrationTimeMicros(
            long featureID, int *errorCode, unsigned long integrationTimeMicros) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setIntegrationTimeMicros(errorCode, integrationTimeMicros);
}

unsigned long DeviceAdapter::spectrometerGetMinimumIntegrationTimeMicros(
            long featureID, int *errorCode) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getMinimumIntegrationTimeMicros(errorCode);
}

unsigned long DeviceAdapter::spectrometerGetMaximumIntegrationTimeMicros(
            long featureID, int *errorCode) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getMaximumIntegrationTimeMicros(errorCode);
}

double DeviceAdapter::spectrometerGetMaximumIntensity(
            long featureID, int *errorCode) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getMaximumIntensity(errorCode);
}

int DeviceAdapter::spectrometerGetUnformattedSpectrumLength(
        long featureID, int *errorCode) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getUnformattedSpectrumLength(errorCode);
}

int DeviceAdapter::spectrometerGetUnformattedSpectrum(long featureID,
        int *errorCode, unsigned char *buffer, int bufferLength) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getUnformattedSpectrum(errorCode, buffer, bufferLength);
}

int DeviceAdapter::spectrometerGetFormattedSpectrumLength(
        long featureID, int *errorCode) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getFormattedSpectrumLength(errorCode);
}

int DeviceAdapter::spectrometerGetFormattedSpectrum(long featureID, int *errorCode,
        double *buffer, int bufferLength) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getFormattedSpectrum(errorCode, buffer, bufferLength);
}

int DeviceAdapter::spectrometerGetWavelengths(long featureID, int *errorCode,
        double *wavelengths, int length) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getWavelengths(errorCode, wavelengths, length);
}

int DeviceAdapter::spectrometerGetElectricDarkPixelCount(
                    long featureID, int *errorCode) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getElectricDarkPixelCount(errorCode);
}

int DeviceAdapter::spectrometerGetElectricDarkPixelIndices(
                    long featureID, int *errorCode, int *indices, int length) {
    SpectrometerFeatureAdapter *feature = getSpectrometerFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getElectricDarkPixelIndices(errorCode, indices, length);
}



/* Pixel binning feature wrappers */
int DeviceAdapter::getNumberOfPixelBinningFeatures() {
    return (int) pixelBinningFeatures.size();
}

int DeviceAdapter::getPixelBinningFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<PixelBinningFeatureAdapter>(
                pixelBinningFeatures, buffer, maxFeatures);
}

void DeviceAdapter::binningSetPixelBinningFactor(long featureID, int *errorCode, const unsigned char binningFactor) {
    PixelBinningFeatureAdapter *feature = getPixelBinningFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
    }

    feature->setPixelBinningFactor(errorCode, binningFactor);
}

unsigned char DeviceAdapter::binningGetPixelBinningFactor(long featureID, int *errorCode) {
    PixelBinningFeatureAdapter *feature = getPixelBinningFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
    }

    return feature->getPixelBinningFactor(errorCode);
}

void DeviceAdapter::binningSetDefaultPixelBinningFactor(long featureID, int *errorCode, const unsigned char binningFactor) {
    PixelBinningFeatureAdapter *feature = getPixelBinningFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
    }

    feature->setDefaultPixelBinningFactor(errorCode, binningFactor);
}

void DeviceAdapter::binningSetDefaultPixelBinningFactor(long featureID, int *errorCode) {
    PixelBinningFeatureAdapter *feature = getPixelBinningFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
    }

    feature->setDefaultPixelBinningFactor(errorCode);
}

unsigned char DeviceAdapter::binningGetDefaultPixelBinningFactor(long featureID, int *errorCode) {
    PixelBinningFeatureAdapter *feature = getPixelBinningFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
    }

    return feature->getDefaultPixelBinningFactor(errorCode);
}

unsigned char DeviceAdapter::binningGetMaxPixelBinningFactor(long featureID, int *errorCode) {
    PixelBinningFeatureAdapter *feature = getPixelBinningFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
    }

    return feature->getMaxPixelBinningFactor(errorCode);
}

/* TEC feature wrappers */
int DeviceAdapter::getNumberOfThermoElectricFeatures() {
    return (int) this->tecFeatures.size();
}

int DeviceAdapter::getThermoElectricFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<ThermoElectricCoolerFeatureAdapter>(
                tecFeatures, buffer, maxFeatures);
}

ThermoElectricCoolerFeatureAdapter *DeviceAdapter::getTECFeatureByID(long featureID) {
    return __getFeatureByID<ThermoElectricCoolerFeatureAdapter>(
                tecFeatures, featureID);
}

double DeviceAdapter::tecReadTemperatureDegreesC(long featureID, int *errorCode) {
    ThermoElectricCoolerFeatureAdapter *feature = getTECFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readTECTemperature(errorCode);
}

void DeviceAdapter::tecSetTemperatureSetpointDegreesC(long featureID, int *errorCode,
        double temperatureDegreesCelsius) {
    ThermoElectricCoolerFeatureAdapter *feature = getTECFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    return feature->setTECTemperature(errorCode, temperatureDegreesCelsius);
}

void DeviceAdapter::tecSetEnable(long featureID, int *errorCode, bool tecEnable) {
    ThermoElectricCoolerFeatureAdapter *feature = getTECFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    return feature->setTECEnable(errorCode, tecEnable);
}

/* Irradiance calibration feature wrappers */
int DeviceAdapter::getNumberOfIrradCalFeatures() {
    return (int) this->irradCalFeatures.size();
}

int DeviceAdapter::getIrradCalFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<IrradCalFeatureAdapter>(
                irradCalFeatures, buffer, maxFeatures);
}

IrradCalFeatureAdapter *DeviceAdapter::getIrradCalFeatureByID(long featureID) {
    return __getFeatureByID<IrradCalFeatureAdapter>(
                irradCalFeatures, featureID);
}

int DeviceAdapter::irradCalibrationRead(long featureID,
        int *errorCode, float *buffer, int bufferLength) {
    IrradCalFeatureAdapter *feature = getIrradCalFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readIrradCalibration(errorCode, buffer, bufferLength);
}

int DeviceAdapter::irradCalibrationWrite(long featureID,
        int *errorCode, float *buffer, int bufferLength) {
    IrradCalFeatureAdapter *feature = getIrradCalFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->writeIrradCalibration(errorCode, buffer, bufferLength);
}

int DeviceAdapter::irradCalibrationHasCollectionArea(long featureID, int *errorCode) {
    IrradCalFeatureAdapter *feature = getIrradCalFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->hasIrradCollectionArea(errorCode);
}

float DeviceAdapter::irradCalibrationReadCollectionArea(long featureID, int *errorCode) {
    IrradCalFeatureAdapter *feature = getIrradCalFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readIrradCollectionArea(errorCode);
}

void DeviceAdapter::irradCalibrationWriteCollectionArea(long featureID,
        int *errorCode, float area) {
    IrradCalFeatureAdapter *feature = getIrradCalFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    return feature->writeIrradCollectionArea(errorCode, area);
}

/* EEPROM feature wrappers */
int DeviceAdapter::getNumberOfEEPROMFeatures() {
    return (int) this->eepromFeatures.size();
}

int DeviceAdapter::getEEPROMFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<EEPROMFeatureAdapter>(
                eepromFeatures, buffer, maxFeatures);
}

EEPROMFeatureAdapter *DeviceAdapter::getEEPROMFeatureByID(long featureID) {
    return __getFeatureByID<EEPROMFeatureAdapter>(
                eepromFeatures, featureID);
}

int DeviceAdapter::eepromReadSlot(long featureID, int *errorCode, int slotNumber,
                    unsigned char *buffer, int length) {
    EEPROMFeatureAdapter *feature = getEEPROMFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readEEPROMSlot(errorCode, slotNumber, buffer, length);
}

/* Light source feature adapters */
int DeviceAdapter::getNumberOfLightSourceFeatures() {
    return (int) this->lightSourceFeatures.size();
}

int DeviceAdapter::getLightSourceFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<LightSourceFeatureAdapter>(
                lightSourceFeatures, buffer, maxFeatures);
}

LightSourceFeatureAdapter *DeviceAdapter::getLightSourceFeatureByID(long featureID) {
    return __getFeatureByID<LightSourceFeatureAdapter>(
                lightSourceFeatures, featureID);
}

int DeviceAdapter::lightSourceGetCount(long featureID, int *errorCode) {

    LightSourceFeatureAdapter *feature = getLightSourceFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getLightSourceCount(errorCode);
}

bool DeviceAdapter::lightSourceHasEnable(long featureID, int *errorCode,
            int lightSourceIndex) {

    LightSourceFeatureAdapter *feature = getLightSourceFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return false;
    }

    return feature->hasLightSourceEnable(errorCode, lightSourceIndex);
}

bool DeviceAdapter::lightSourceIsEnabled(long featureID, int *errorCode,
            int lightSourceIndex) {

    LightSourceFeatureAdapter *feature = getLightSourceFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return false;
    }

    return feature->isLightSourceEnabled(errorCode, lightSourceIndex);
}

void DeviceAdapter::lightSourceSetEnable(long featureID, int *errorCode,
            int lightSourceIndex, bool enable) {

    LightSourceFeatureAdapter *feature = getLightSourceFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setLightSourceEnable(errorCode, lightSourceIndex, enable);
}

bool DeviceAdapter::lightSourceHasVariableIntensity(long featureID, int *errorCode,
            int lightSourceIndex) {

    LightSourceFeatureAdapter *feature = getLightSourceFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return false;
    }

    return feature->hasVariableIntensity(errorCode, lightSourceIndex);
}

double DeviceAdapter::lightSourceGetIntensity(long featureID, int *errorCode,
            int lightSourceIndex) {

    LightSourceFeatureAdapter *feature = getLightSourceFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0.0;
    }

    return feature->getLightSourceIntensity(errorCode, lightSourceIndex);
}

void DeviceAdapter::lightSourceSetIntensity(long featureID, int *errorCode,
            int lightSourceIndex, double intensity) {

    LightSourceFeatureAdapter *feature = getLightSourceFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setLightSourceIntensity(errorCode, lightSourceIndex, intensity);
}

/* Strobe/lamp feature adapters */
int DeviceAdapter::getNumberOfStrobeLampFeatures() {
    return (int) this->strobeLampFeatures.size();
}

int DeviceAdapter::getStrobeLampFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<StrobeLampFeatureAdapter>(
                strobeLampFeatures, buffer, maxFeatures);
}

StrobeLampFeatureAdapter *DeviceAdapter::getStrobeLampFeatureByID(long featureID) {
    return __getFeatureByID<StrobeLampFeatureAdapter>(
                strobeLampFeatures, featureID);
}

void DeviceAdapter::lampSetStrobeEnable(long featureID,
            int *errorCode, bool strobeEnable) {
    StrobeLampFeatureAdapter *feature = getStrobeLampFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setStrobeLampEnable(errorCode, strobeEnable);
}

/* Continuous strobe feature adapters */
int DeviceAdapter::getNumberOfContinuousStrobeFeatures() {
    return (int) this->continuousStrobeFeatures.size();
}

int DeviceAdapter::getContinuousStrobeFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<ContinuousStrobeFeatureAdapter>(
                continuousStrobeFeatures, buffer, maxFeatures);
}

ContinuousStrobeFeatureAdapter *DeviceAdapter::getContinuousStrobeFeatureByID(long featureID) {
    return __getFeatureByID<ContinuousStrobeFeatureAdapter>(
                continuousStrobeFeatures, featureID);
}

void DeviceAdapter::continuousStrobeSetPeriodMicroseconds(long featureID,
            int *errorCode, unsigned long period_usec) {
    ContinuousStrobeFeatureAdapter *feature = getContinuousStrobeFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setContinuousStrobePeriodMicroseconds(errorCode, period_usec);
}

void DeviceAdapter::continuousStrobeSetEnable(long featureID,
            int *errorCode, bool enable) {
    ContinuousStrobeFeatureAdapter *feature = getContinuousStrobeFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setContinuousStrobeEnable(errorCode, enable);
}

/* Shutter feature wrappers */
int DeviceAdapter::getNumberOfShutterFeatures() {
    return (int) this->shutterFeatures.size();
}

int DeviceAdapter::getShutterFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<ShutterFeatureAdapter>(
                shutterFeatures, buffer, maxFeatures);
}

ShutterFeatureAdapter *DeviceAdapter::getShutterFeatureByID(long featureID) {
    return __getFeatureByID<ShutterFeatureAdapter>(
                shutterFeatures, featureID);
}

void DeviceAdapter::shutterSetShutterOpen(long featureID,
            int *errorCode, bool opened) {
    ShutterFeatureAdapter *feature = getShutterFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setShutterOpen(errorCode, opened);
}


/* Nonlinearity coefficients feature wrappers */
int DeviceAdapter::getNumberOfNonlinearityCoeffsFeatures() {
    return (int) this->nonlinearityFeatures.size();
}

int DeviceAdapter::getNonlinearityCoeffsFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<NonlinearityCoeffsFeatureAdapter>(
                nonlinearityFeatures, buffer, maxFeatures);
}

NonlinearityCoeffsFeatureAdapter *DeviceAdapter::getNonlinearityCoeffsFeatureByID(long featureID) {
    return __getFeatureByID<NonlinearityCoeffsFeatureAdapter>(
                nonlinearityFeatures, featureID);
}

int DeviceAdapter::nonlinearityCoeffsGet(long featureID, int *errorCode,
        double *buffer, int bufferLength) {
    NonlinearityCoeffsFeatureAdapter *feature = getNonlinearityCoeffsFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readNonlinearityCoeffs(errorCode, buffer, bufferLength);
}

/* Temperature feature wrappers */
int DeviceAdapter::getNumberOfTemperatureFeatures() {
    return (int) this->temperatureFeatures.size();
}

int DeviceAdapter::getTemperatureFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<TemperatureFeatureAdapter>(
                temperatureFeatures, buffer, maxFeatures);
}

TemperatureFeatureAdapter *DeviceAdapter::getTemperatureFeatureByID(long temperatureFeatureID) {
    return __getFeatureByID<TemperatureFeatureAdapter>(
                temperatureFeatures, temperatureFeatureID);
}

unsigned char DeviceAdapter::temperatureCountGet(long temperatureFeatureID, int *errorCode) {
    TemperatureFeatureAdapter *feature = getTemperatureFeatureByID(temperatureFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readTemperatureCount(errorCode);
}

double DeviceAdapter::temperatureGet(long temperatureFeatureID, int *errorCode, int index) {
    TemperatureFeatureAdapter *feature = getTemperatureFeatureByID(temperatureFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readTemperature(errorCode, index);
}

int DeviceAdapter::temperatureGetAll(long temperatureFeatureID, int *errorCode,
        double *buffer, int bufferLength) {
    TemperatureFeatureAdapter *feature = getTemperatureFeatureByID(temperatureFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readAllTemperatures(errorCode, buffer, bufferLength);
}

/* Revision feature wrappers */
int DeviceAdapter::getNumberOfRevisionFeatures() {
    return (int) this->revisionFeatures.size();
}

int DeviceAdapter::getRevisionFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<RevisionFeatureAdapter>(
                revisionFeatures, buffer, maxFeatures);
}

RevisionFeatureAdapter *DeviceAdapter::getRevisionFeatureByID(long revisionFeatureID) {
    return __getFeatureByID<RevisionFeatureAdapter>(
                revisionFeatures, revisionFeatureID);
}

unsigned char DeviceAdapter::revisionHardwareGet(long revisionFeatureID, int *errorCode) {
    RevisionFeatureAdapter *feature = getRevisionFeatureByID(revisionFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readHardwareRevision(errorCode);
}

unsigned short int DeviceAdapter::revisionFirmwareGet(long revisionFeatureID, int *errorCode) {
    RevisionFeatureAdapter *feature = getRevisionFeatureByID(revisionFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readFirmwareRevision(errorCode);
}

/* Optical Bench feature wrappers */
int DeviceAdapter::getNumberOfOpticalBenchFeatures() {
    return (int) this->opticalBenchFeatures.size();
}

int DeviceAdapter::getOpticalBenchFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<OpticalBenchFeatureAdapter>(
                opticalBenchFeatures, buffer, maxFeatures);
}

OpticalBenchFeatureAdapter *DeviceAdapter::getOpticalBenchFeatureByID(long opticalBenchFeatureID) {
    return __getFeatureByID<OpticalBenchFeatureAdapter>(
               opticalBenchFeatures, opticalBenchFeatureID);
}

unsigned short int DeviceAdapter::opticalBenchGetFiberDiameterMicrons(long opticalBenchFeatureID, int *errorCode) {
    OpticalBenchFeatureAdapter *feature = getOpticalBenchFeatureByID(opticalBenchFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readOpticalBenchFiberDiameterMicrons(errorCode);
}

unsigned short int DeviceAdapter::opticalBenchGetSlitWidthMicrons(long opticalBenchFeatureID, int *errorCode) {
    OpticalBenchFeatureAdapter *feature = getOpticalBenchFeatureByID(opticalBenchFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readOpticalBenchSlitWidthMicrons(errorCode);
}

int DeviceAdapter::opticalBenchGetID(long featureID, int *errorCode,
        char *buffer, int bufferLength) {
    OpticalBenchFeatureAdapter *feature = getOpticalBenchFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }
    return feature->readOpticalBenchID(errorCode, buffer, bufferLength);
}

int DeviceAdapter::opticalBenchGetSerialNumber(long featureID, int *errorCode,
        char *buffer, int bufferLength) {
    OpticalBenchFeatureAdapter *feature = getOpticalBenchFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readOpticalBenchSerialNumber(errorCode, buffer, bufferLength);
}

int DeviceAdapter::opticalBenchGetCoating(long featureID, int *errorCode,
        char *buffer, int bufferLength) {
    OpticalBenchFeatureAdapter *feature = getOpticalBenchFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readOpticalBenchCoating(errorCode, buffer, bufferLength);
}

int DeviceAdapter::opticalBenchGetFilter(long featureID, int *errorCode,
        char *buffer, int bufferLength) {
    OpticalBenchFeatureAdapter *feature = getOpticalBenchFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readOpticalBenchFilter(errorCode, buffer, bufferLength);
}

int DeviceAdapter::opticalBenchGetGrating(long featureID, int *errorCode,
        char *buffer, int bufferLength) {
    OpticalBenchFeatureAdapter *feature = getOpticalBenchFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readOpticalBenchGrating(errorCode, buffer, bufferLength);
}

/* Spectrum processing feature wrappers */
int DeviceAdapter::getNumberOfSpectrumProcessingFeatures() {
    return (int) this->spectrumProcessingFeatures.size();
}

int DeviceAdapter::getSpectrumProcessingFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<SpectrumProcessingFeatureAdapter>(
                spectrumProcessingFeatures, buffer, maxFeatures);
}

SpectrumProcessingFeatureAdapter *DeviceAdapter::getSpectrumProcessingFeatureByID(long spectrumProcessingFeatureID) {
    return __getFeatureByID<SpectrumProcessingFeatureAdapter>(
               spectrumProcessingFeatures, spectrumProcessingFeatureID);
}

unsigned short int DeviceAdapter::spectrumProcessingScansToAverageGet(long spectrumProcessingFeatureID, int *errorCode) {
    SpectrumProcessingFeatureAdapter *feature = getSpectrumProcessingFeatureByID(spectrumProcessingFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readSpectrumProcessingScansToAverage(errorCode);
}

unsigned char DeviceAdapter::spectrumProcessingBoxcarWidthGet(long spectrumProcessingFeatureID, int *errorCode) {
    SpectrumProcessingFeatureAdapter *feature = getSpectrumProcessingFeatureByID(spectrumProcessingFeatureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readSpectrumProcessingBoxcarWidth(errorCode);
}

void DeviceAdapter::spectrumProcessingBoxcarWidthSet(long featureID, int *errorCode,
            unsigned char boxcarWidth) {

    SpectrumProcessingFeatureAdapter *feature = getSpectrumProcessingFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->writeSpectrumProcessingBoxcarWidth(errorCode, boxcarWidth);
}

void DeviceAdapter::spectrumProcessingScansToAverageSet(long featureID, int *errorCode,
            unsigned short int scansToAverage) {

    SpectrumProcessingFeatureAdapter *feature = getSpectrumProcessingFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->writeSpectrumProcessingScansToAverage(errorCode, scansToAverage);
}

/* Stray light coefficients feature wrappers */
int DeviceAdapter::getNumberOfStrayLightCoeffsFeatures() {
    return (int) this->strayLightFeatures.size();
}

int DeviceAdapter::getStrayLightCoeffsFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<StrayLightCoeffsFeatureAdapter>(
                strayLightFeatures, buffer, maxFeatures);
}


StrayLightCoeffsFeatureAdapter *DeviceAdapter::getStrayLightCoeffsFeatureByID(long featureID) {
    return __getFeatureByID<StrayLightCoeffsFeatureAdapter>(
                strayLightFeatures, featureID);
}

int DeviceAdapter::strayLightCoeffsGet(long featureID, int *errorCode,
        double *buffer, int bufferLength) {
    StrayLightCoeffsFeatureAdapter *feature = getStrayLightCoeffsFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->readStrayLightCoeffs(errorCode, buffer, bufferLength);
}

PixelBinningFeatureAdapter *DeviceAdapter::getPixelBinningFeatureByID(long featureID) {
    return __getFeatureByID<PixelBinningFeatureAdapter>(pixelBinningFeatures, featureID);
}
/* Data buffer feature wrappers */
int DeviceAdapter::getNumberOfDataBufferFeatures() {
    return (int) this->dataBufferFeatures.size();
}

int DeviceAdapter::getDataBufferFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<DataBufferFeatureAdapter>(
                dataBufferFeatures, buffer, maxFeatures);
}

DataBufferFeatureAdapter *DeviceAdapter::getDataBufferFeatureByID(long featureID) {
    return __getFeatureByID<DataBufferFeatureAdapter>(
                dataBufferFeatures, featureID);
}

void DeviceAdapter::dataBufferClear(long featureID, int *errorCode) {
    DataBufferFeatureAdapter *feature = getDataBufferFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->clearBuffer(errorCode);
}

unsigned long DeviceAdapter::dataBufferGetNumberOfElements(long featureID, int *errorCode) {
    DataBufferFeatureAdapter *feature = getDataBufferFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getNumberOfElements(errorCode);
}

unsigned long DeviceAdapter::dataBufferGetBufferCapacity(long featureID, int *errorCode) {
    DataBufferFeatureAdapter *feature = getDataBufferFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getBufferCapacity(errorCode);
}

unsigned long DeviceAdapter::dataBufferGetBufferCapacityMaximum(long featureID, int *errorCode) {
    DataBufferFeatureAdapter *feature = getDataBufferFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getBufferCapacityMaximum(errorCode);
}

unsigned long DeviceAdapter::dataBufferGetBufferCapacityMinimum(long featureID, int *errorCode) {
    DataBufferFeatureAdapter *feature = getDataBufferFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getBufferCapacityMinimum(errorCode);
}

void DeviceAdapter::dataBufferSetBufferCapacity(long featureID, int *errorCode, unsigned long capacity) {
    DataBufferFeatureAdapter *feature = getDataBufferFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setBufferCapacity(errorCode, capacity);
}

/* Acquisition delay feature wrappers */
int DeviceAdapter::getNumberOfAcquisitionDelayFeatures() {
    return (int) this->acquisitionDelayFeatures.size();
}

int DeviceAdapter::getAcquisitionDelayFeatures(long *buffer, int maxFeatures) {
    return __getFeatureIDs<AcquisitionDelayFeatureAdapter>(
                acquisitionDelayFeatures, buffer, maxFeatures);
}

AcquisitionDelayFeatureAdapter *DeviceAdapter::getAcquisitionDelayFeatureByID(long featureID) {
    return __getFeatureByID<AcquisitionDelayFeatureAdapter>(
                acquisitionDelayFeatures, featureID);
}

void DeviceAdapter::acquisitionDelaySetDelayMicroseconds(long featureID, int *errorCode,
        unsigned long delay_usec) {
    AcquisitionDelayFeatureAdapter *feature = getAcquisitionDelayFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return;
    }

    feature->setAcquisitionDelayMicroseconds(errorCode, delay_usec);
}

unsigned long DeviceAdapter::acquisitionDelayGetDelayMicroseconds(long featureID, int *errorCode) {
    AcquisitionDelayFeatureAdapter *feature = getAcquisitionDelayFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getAcquisitionDelayMicroseconds(errorCode);
}

unsigned long DeviceAdapter::acquisitionDelayGetDelayIncrementMicroseconds(long featureID, int *errorCode) {
    AcquisitionDelayFeatureAdapter *feature = getAcquisitionDelayFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getAcquisitionDelayIncrementMicroseconds(errorCode);
}

unsigned long DeviceAdapter::acquisitionDelayGetDelayMaximumMicroseconds(long featureID, int *errorCode) {
    AcquisitionDelayFeatureAdapter *feature = getAcquisitionDelayFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getAcquisitionDelayMaximumMicroseconds(errorCode);
}

unsigned long DeviceAdapter::acquisitionDelayGetDelayMinimumMicroseconds(long featureID, int *errorCode) {
    AcquisitionDelayFeatureAdapter *feature = getAcquisitionDelayFeatureByID(featureID);
    if(NULL == feature) {
        SET_ERROR_CODE(ERROR_FEATURE_NOT_FOUND);
        return 0;
    }

    return feature->getAcquisitionDelayMinimumMicroseconds(errorCode);
}

