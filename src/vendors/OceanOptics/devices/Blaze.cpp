/***************************************************//**
 * @file    Blaze.cpp
 * @date    February 2016
 * @author  Ocean Optics, Inc.
 *
 * LICENSE:
 *
 * SeaBreeze Copyright (C) 2016, Ocean Optics Inc
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
#include "api/seabreezeapi/ProtocolFamilies.h"
#include "common/buses/BusFamilies.h"
#include "vendors/OceanOptics/buses/network/BlazeTCPIPv4.h"
#include "vendors/OceanOptics/buses/usb/BlazeUSB.h"
#include "vendors/OceanOptics/devices/Blaze.h"
#include "vendors/OceanOptics/features/light_source/StrobeLampFeature.h"
#include "vendors/OceanOptics/features/raw_bus_access/RawUSBBusAccessFeature.h"
#include "vendors/OceanOptics/features/serial_number/SerialNumberFeature.h"
#include "vendors/OceanOptics/features/spectrometer/ProgrammableSaturationFeatureImpl.h"
#include "vendors/OceanOptics/features/nonlinearity/NonlinearityCoeffsFeature.h"
#include "vendors/OceanOptics/features/stray_light/StrayLightCoeffsFeature.h"
#include "vendors/OceanOptics/features/spectrometer/BlazeSpectrometerFeature.h"
#include "vendors/OceanOptics/protocols/obp/impls/OBPProgrammableSaturationProtocol.h"
#include "vendors/OceanOptics/protocols/obp/impls/OBPSerialNumberProtocol.h"
#include "vendors/OceanOptics/protocols/obp/impls/OBPNonlinearityCoeffsProtocol.h"
#include "vendors/OceanOptics/protocols/obp/impls/OBPStrayLightCoeffsProtocol.h"
#include "vendors/OceanOptics/protocols/obp/impls/OBPStrobeLampProtocol.h"
#include "vendors/OceanOptics/protocols/obp/impls/OceanBinaryProtocol.h"

using namespace seabreeze;
using namespace seabreeze::oceanBinaryProtocol;
using namespace seabreeze::api;
using namespace std;

Blaze::Blaze() {

    this->name = "Blaze";
        
    // 0 is the control address, since it is not valid in this context, means not used
    this->usbEndpoint_primary_out = 0x01;
    this->usbEndpoint_primary_in = 0x81;
    this->usbEndpoint_secondary_out = 0x02;
    this->usbEndpoint_secondary_in = 0x82;
    this->usbEndpoint_secondary_in2 = 0;

    /* Set up the available buses on this device */
    this->buses.push_back(new BlazeUSB());
    this->buses.push_back(new BlazeTCPIPv4());

    /* Set up the available protocols understood by this device */
    this->protocols.push_back(new OceanBinaryProtocol());

    /* Set up the features that comprise this device */
    
    vector<ProtocolHelper *> saturationHelpers;
    saturationHelpers.push_back(new OBPProgrammableSaturationProtocol());
    ProgrammableSaturationFeature *saturation =
            new ProgrammableSaturationFeatureImpl(saturationHelpers);
    
    this->features.push_back(new BlazeSpectrometerFeature(saturation));

    /* Add serial number feature */
    vector<ProtocolHelper *> serialNumberHelpers;
    serialNumberHelpers.push_back(new OBPSerialNumberProtocol());
    this->features.push_back(new SerialNumberFeature(serialNumberHelpers));

    /* Add nonlinearity coefficients feature */
    vector<ProtocolHelper *> nonlinearityHelpers;
    nonlinearityHelpers.push_back(new OBPNonlinearityCoeffsProtocol());
    this->features.push_back(
        new NonlinearityCoeffsFeature(nonlinearityHelpers));

    /* Add stray light coefficients feature */
    vector<ProtocolHelper *> strayHelpers;
    strayHelpers.push_back(new OBPStrayLightCoeffsProtocol());
    this->features.push_back(new StrayLightCoeffsFeature(strayHelpers));
    
    /* Add lamp enable feature */
    vector<ProtocolHelper *> lampHelpers;
    lampHelpers.push_back(new OBPStrobeLampProtocol());
    this->features.push_back(new StrobeLampFeature(lampHelpers));

    this->features.push_back(new RawUSBBusAccessFeature());
}

Blaze::~Blaze() {
}

ProtocolFamily Blaze::getSupportedProtocol(FeatureFamily family, BusFamily bus) {
    ProtocolFamilies protocols;

    /* The Blaze uses one protocol for all buses. */
    return protocols.OCEAN_BINARY_PROTOCOL;
}
