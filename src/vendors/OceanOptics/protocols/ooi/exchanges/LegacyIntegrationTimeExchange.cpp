/***************************************************//**
 * @file    LegacyIntegrationTimeExchange.cpp
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
#include "vendors/OceanOptics/protocols/ooi/exchanges/LegacyIntegrationTimeExchange.h"
#include "vendors/OceanOptics/protocols/ooi/hints/ControlHint.h"
#include "vendors/OceanOptics/protocols/ooi/constants/OpCodes.h"

using namespace seabreeze;
using namespace seabreeze::ooiProtocol;

LegacyIntegrationTimeExchange::LegacyIntegrationTimeExchange(
        unsigned long intTimeBase_usec) : IntegrationTimeExchange(intTimeBase_usec) {

    this->buffer->resize(3);
    this->length = 3;
    checkBufferSize();
}

LegacyIntegrationTimeExchange::~LegacyIntegrationTimeExchange() {

}

Data *LegacyIntegrationTimeExchange::transfer(TransferHelper *helper)
        throw (ProtocolException) {
    /* Note that it might be cleaner to populate the buffer when the integration
     * time is first written in, but this guarantees that it only happens once
     * per transfer.
     */

    /* Start with the protocol command number */
    (*(this->buffer))[0] = OpCodes::OP_ITIME;

    /* Then fill in the integration time, 2 bytes, MSB first. */
    (*(this->buffer))[1] = (byte)((this->integrationTime_usec & 0x000000FF00) >> 8);
    (*(this->buffer))[2] = (byte)(this->integrationTime_usec & 0x00000000FF);

    /* Now delegate to the superclass to move the buffer. */
    return Transfer::transfer(helper);
}
