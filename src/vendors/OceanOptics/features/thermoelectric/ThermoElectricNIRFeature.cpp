/***************************************************//**
 * @file    ThermoElectricNIRFeature.cpp
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
#include "vendors/OceanOptics/features/thermoelectric/ThermoElectricNIRFeature.h"
#include "vendors/OceanOptics/protocols/interfaces/ThermoElectricProtocolInterface.h"
#include "vendors/OceanOptics/protocols/ooi/impls/OOITECProtocol.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/TECNIREnableExchange.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/WriteTECNIRSetPointExchange.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/ReadTECNIRTemperatureExchange.h"

using namespace seabreeze;
using namespace seabreeze::ooiProtocol;
using namespace std;

ThermoElectricNIRFeature::ThermoElectricNIRFeature() {

    TECEnableExchange *enableExchange = new TECNIREnableExchange();
    WriteTECSetPointExchange *writeSetPointExchange = new WriteTECNIRSetPointExchange();
    ReadTECTemperatureExchange *readTECTempExchange = new ReadTECNIRTemperatureExchange();

    OOITECProtocol *ooiProtocol =
            new OOITECProtocol(enableExchange, writeSetPointExchange,
                readTECTempExchange);

    this->protocols.push_back(ooiProtocol);
}

ThermoElectricNIRFeature::~ThermoElectricNIRFeature() {

}
