/***************************************************//**
 * @file    OOIPCBTemperatureProtocol.cpp
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
#include <string>
#include "vendors/OceanOptics/protocols/ooi/impls/OOIPCBTemperatureProtocol.h"
#include "vendors/OceanOptics/protocols/ooi/exchanges/PCBTemperatureExchange.h"
#include "vendors/OceanOptics/protocols/ooi/impls/OOIProtocol.h"
#include "common/DoubleVector.h"
#include "common/exceptions/ProtocolBusMismatchException.h"

using namespace seabreeze;
using namespace seabreeze::ooiProtocol;
using namespace std;

OOIPCBTemperatureProtocol::OOIPCBTemperatureProtocol()
        : TemperatureProtocolInterface(new OOIProtocol()) {

}

OOIPCBTemperatureProtocol::~OOIPCBTemperatureProtocol() {

}


unsigned char OOIPCBTemperatureProtocol::readTemperatureCount(const Bus &bus)
                throw (ProtocolException)
{
    return 1;
}

double OOIPCBTemperatureProtocol::readTemperature(const Bus &bus, int index)
                throw (ProtocolException)
{

    PCBTemperatureExchange xchange;

    TransferHelper *helper = bus.getHelper(xchange.getHints());
    if(NULL == helper)
    {
        string error("Failed to find a helper to bridge given protocol and bus.");
        throw ProtocolBusMismatchException(error);
    }

    // Check index
    if(index != 0)
    {
        // Only one index is supported
        return 0;
    }

    Data *result;
    result = xchange.transfer(helper);

    // Cast to DoubleVector
    DoubleVector* temp;

    temp = dynamic_cast<DoubleVector*>(result);
    if(NULL == temp)
    {
        string error("Expected Transfer::transfer to produce a non-null result "
                "containing raw EEPROM data.  Without this data, it is not possible to "
                "generate a valid EEPROM slot value.");
        throw ProtocolException(error);
    }

    return temp->getDoubleVector()[0];
}


vector<double> *OOIPCBTemperatureProtocol::readAllTemperatures(const Bus &bus)
        throw (ProtocolException) {

    PCBTemperatureExchange xchange;

    TransferHelper *helper = bus.getHelper(xchange.getHints());
    if(NULL == helper)
    {
        string error("Failed to find a helper to bridge given protocol and bus.");
        throw ProtocolBusMismatchException(error);
    }

    Data *result;
    result = xchange.transfer(helper);

    // Cast to DoubleVector
    DoubleVector* temp;

    temp = dynamic_cast<DoubleVector*>(result);
    if(NULL == temp)
    {
        string error("Expected Transfer::transfer to produce a non-null result "
                "containing raw EEPROM data.  Without this data, it is not possible to "
                "generate a valid EEPROM slot value.");
        throw ProtocolException(error);
    }

    vector<double>* retval = new vector<double>(1); // temperature array to be returned
    (*retval)[0] = temp->getDoubleVector()[0];
    return retval;
}
