/***************************************************//**
 * @file    ThermoElectricNIRFeature.h
 * @date    March 2020
 * @author  Michele Devetta
 *
 * This feature provides an interface to the thermo-
 * electric cooler (TEC) on the NIR256 and NIR512
 * spectrometers.
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

#ifndef THERMOELECTRICNIRFEATURE_H
#define THERMOELECTRICNIRFEATURE_H

#include "common/buses/Bus.h"
#include "common/protocols/Protocol.h"
#include "common/features/Feature.h"
#include "vendors/OceanOptics/features/thermoelectric/ThermoElectricFeatureBase.h"

namespace seabreeze {

    class ThermoElectricNIRFeature : public ThermoElectricFeatureBase {
    public:
        ThermoElectricNIRFeature();
        virtual ~ThermoElectricNIRFeature();

        virtual double getDefaultSetPointCelsius(const Protocol &protocol, const Bus &bus) throw (FeatureException) { return 0.0; };
        virtual bool getDefaultThermoElectricEnable(const Protocol &protocol, const Bus &bus) throw (FeatureException) { return true; };
        virtual bool initialize(const Protocol &protocol, const Bus &bus) throw (FeatureException) { return true; };
    };
}

#endif /* THERMOELECTRICNIRFEATURE_H */
