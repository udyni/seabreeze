/***************************************************//**
 * @file    NIR512SpectrometerFeature.cpp
 * @date    March 2020
 * @author  Michele Devetta
 *
 * LICENSE:
 *
 * SeaBreeze Copyright (C) 2020, Michele Devetta
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
#include "vendors/OceanOptics/features/spectrometer/NIR512SpectrometerFeature.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/LegacyIntegrationTimeExchange.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/LegacyNIRSpectrumExchange.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/RequestSpectrumExchange.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/TriggerModeExchange.h"
#include "vendors/OceanOptics/protocols/ooi/impls/OOISpectrometerProtocol.h"

using namespace seabreeze;
using namespace ooiProtocol;

const long NIR512SpectrometerFeature::INTEGRATION_TIME_MINIMUM = 1000;
const long NIR512SpectrometerFeature::INTEGRATION_TIME_MAXIMUM = 65535000;
const long NIR512SpectrometerFeature::INTEGRATION_TIME_INCREMENT = 1000;
const long NIR512SpectrometerFeature::INTEGRATION_TIME_BASE = 1000;

NIR512SpectrometerFeature::NIR512SpectrometerFeature() {

    int readoutLength = 512 * 2 + 1;

    this->numberOfPixels = 512;
    this->numberOfBytesPerPixel = sizeof(unsigned short);
    this->maxIntensity = 65535;

    this->integrationTimeMinimum = NIR512SpectrometerFeature::INTEGRATION_TIME_MINIMUM;
    this->integrationTimeMaximum = NIR512SpectrometerFeature::INTEGRATION_TIME_MAXIMUM;
    this->integrationTimeBase = NIR512SpectrometerFeature::INTEGRATION_TIME_BASE;
    this->integrationTimeIncrement = NIR512SpectrometerFeature::INTEGRATION_TIME_INCREMENT;

    LegacyIntegrationTimeExchange *intTime = new LegacyIntegrationTimeExchange(NIR512SpectrometerFeature::INTEGRATION_TIME_BASE);

    Transfer *requestFormattedSpectrum = new RequestSpectrumExchange();
    Transfer *readFormattedSpectrum = new LegacyNIRSpectrumExchange(readoutLength, this->numberOfPixels);
    Transfer *requestUnformattedSpectrum = new RequestSpectrumExchange();
    Transfer *readUnformattedSpectrum = new LegacyNIRSpectrumExchange(readoutLength, this->numberOfPixels);
    Transfer *requestFastBufferSpectrum = new RequestSpectrumExchange();
    Transfer *readFastBufferSpectrum = new LegacyNIRSpectrumExchange(readoutLength, this->numberOfPixels);

    TriggerModeExchange *triggerMode = new TriggerModeExchange();

    OOISpectrometerProtocol *ooiProtocol = new OOISpectrometerProtocol(intTime, requestFormattedSpectrum, readFormattedSpectrum, 
        requestUnformattedSpectrum, readUnformattedSpectrum, requestFastBufferSpectrum, readFastBufferSpectrum, triggerMode);
    this->protocols.push_back(ooiProtocol);

    this->triggerModes.push_back(
        new SpectrometerTriggerMode(SPECTROMETER_TRIGGER_MODE_NORMAL));
    this->triggerModes.push_back(
        new SpectrometerTriggerMode(SPECTROMETER_TRIGGER_MODE_SOFTWARE));
}

NIR512SpectrometerFeature::~NIR512SpectrometerFeature() {

}
