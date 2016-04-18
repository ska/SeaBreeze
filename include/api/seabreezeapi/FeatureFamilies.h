/***************************************************//**
 * @file    FeatureFamilies.h
 * @date    February 2015
 * @author  Ocean Optics, Inc., Kirk Clendinning, Heliospectra
 *
 * This provides a way to get references to different kinds
 * of features (e.g. spectrometer, TEC) generically.
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

#ifndef SEABREEZE_FEATUREFAMILIES_H
#define SEABREEZE_FEATUREFAMILIES_H

#include "common/features/FeatureFamily.h"
#include <vector>

namespace seabreeze {
    namespace api {

        class UndefinedFeatureFamily : public FeatureFamily {
        public:
            UndefinedFeatureFamily();
            virtual ~UndefinedFeatureFamily();
        };

        class SerialNumberFeatureFamily : public FeatureFamily {
        public:
            SerialNumberFeatureFamily();
            virtual ~SerialNumberFeatureFamily();
        };

        class SpectrometerFeatureFamily : public FeatureFamily {
        public:
            SpectrometerFeatureFamily();
            virtual ~SpectrometerFeatureFamily();
        };

        class ThermoElectricFeatureFamily : public FeatureFamily {
        public:
            ThermoElectricFeatureFamily();
            virtual ~ThermoElectricFeatureFamily();
        };

        class IrradCalFeatureFamily : public FeatureFamily {
        public:
            IrradCalFeatureFamily();
            virtual ~IrradCalFeatureFamily();
        };

        class EEPROMFeatureFamily : public FeatureFamily {
        public:
            EEPROMFeatureFamily();
            virtual ~EEPROMFeatureFamily();
        };

        class LightSourceFeatureFamily : public FeatureFamily {
        public:
            LightSourceFeatureFamily();
            virtual ~LightSourceFeatureFamily();
        };

        class StrobeLampFeatureFamily : public FeatureFamily {
        public:
            StrobeLampFeatureFamily();
            virtual ~StrobeLampFeatureFamily();
        };

        class ContinuousStrobeFeatureFamily : public FeatureFamily {
        public:
            ContinuousStrobeFeatureFamily();
            virtual ~ContinuousStrobeFeatureFamily();
        };

        class ShutterFeatureFamily : public FeatureFamily {
        public:
            ShutterFeatureFamily();
            virtual ~ShutterFeatureFamily();
        };

        class WaveCalFeatureFamily : public FeatureFamily {
        public:
            WaveCalFeatureFamily();
            virtual ~WaveCalFeatureFamily();
        };

        class NonlinearityCoeffsFeatureFamily : public FeatureFamily {
        public:
            NonlinearityCoeffsFeatureFamily();
            virtual ~NonlinearityCoeffsFeatureFamily();
        };

        class TemperatureFeatureFamily : public FeatureFamily {
        public:
            TemperatureFeatureFamily();
            virtual ~TemperatureFeatureFamily();
        };

        class RevisionFeatureFamily : public FeatureFamily {
        public:
            RevisionFeatureFamily();
            virtual ~RevisionFeatureFamily();
        };
        
        class OpticalBenchFeatureFamily : public FeatureFamily {
        public:
            OpticalBenchFeatureFamily();
            virtual ~OpticalBenchFeatureFamily();
        };
        
        class SpectrumProcessingFeatureFamily : public FeatureFamily {
        public:
            SpectrumProcessingFeatureFamily();
            virtual ~SpectrumProcessingFeatureFamily();
        };

        class StrayLightCoeffsFeatureFamily : public FeatureFamily {
        public:
            StrayLightCoeffsFeatureFamily();
            virtual ~StrayLightCoeffsFeatureFamily();
        };
        
        class RawUSBBusAccessFeatureFamily : public FeatureFamily {
        public:
            RawUSBBusAccessFeatureFamily();
            virtual ~RawUSBBusAccessFeatureFamily();
        };
        
        class DataBufferFeatureFamily : public FeatureFamily {
        public:
            DataBufferFeatureFamily();
            virtual ~DataBufferFeatureFamily();
        };

        class AcquisitionDelayFeatureFamily : public FeatureFamily {
        public:
            AcquisitionDelayFeatureFamily();
            virtual ~AcquisitionDelayFeatureFamily();
        };

        class PixelBinningFeatureFamily : public FeatureFamily {
        public:
            PixelBinningFeatureFamily();
            virtual ~PixelBinningFeatureFamily();
        };

        class FeatureFamilies {
        public:
            const UndefinedFeatureFamily UNDEFINED;
            const SerialNumberFeatureFamily SERIAL_NUMBER;
            const SpectrometerFeatureFamily SPECTROMETER;
            const ThermoElectricFeatureFamily THERMOELECTRIC;
            const IrradCalFeatureFamily IRRAD_CAL;
            const EEPROMFeatureFamily EEPROM;
            const LightSourceFeatureFamily LIGHT_SOURCE;
            const StrobeLampFeatureFamily STROBE_LAMP_ENABLE;
            const ContinuousStrobeFeatureFamily CONTINUOUS_STROBE;
            const ShutterFeatureFamily SHUTTER;
            const WaveCalFeatureFamily WAVELENGTH_CAL;
            const NonlinearityCoeffsFeatureFamily NONLINEARITY_COEFFS;
            const TemperatureFeatureFamily TEMPERATURE;
            const RevisionFeatureFamily REVISION;
            const OpticalBenchFeatureFamily OPTICAL_BENCH;
            const SpectrumProcessingFeatureFamily SPECTRUM_PROCESSING;
            const StrayLightCoeffsFeatureFamily STRAY_LIGHT_COEFFS;
            const RawUSBBusAccessFeatureFamily RAW_USB_BUS_ACCESS;
            const DataBufferFeatureFamily DATA_BUFFER;
            const AcquisitionDelayFeatureFamily ACQUISITION_DELAY;
            const PixelBinningFeatureFamily PIXEL_BINNING;

            FeatureFamilies();
            ~FeatureFamilies();
            std::vector<FeatureFamily *> getAllFeatureFamilies();
        };
    }
}

#endif

