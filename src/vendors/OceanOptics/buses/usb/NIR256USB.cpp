/***************************************************//**
 * @file    NIR256USB.cpp
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
#include "vendors/OceanOptics/buses/usb/NIR256USB.h"
#include "vendors/OceanOptics/buses/usb/OOIUSBProductID.h"
#include "vendors/OceanOptics/buses/usb/OOIUSBEndpointMaps.h"
#include "vendors/OceanOptics/protocols/ooi/hints/ControlHint.h"
#include "vendors/OceanOptics/protocols/ooi/hints/SpectrumHint.h"
#include "vendors/OceanOptics/buses/usb/OOIUSBControlTransferHelper.h"
#include "vendors/OceanOptics/buses/usb/OOIUSBSpectrumTransferHelper.h"

using namespace seabreeze;
using namespace ooiProtocol;

NIR256USB::NIR256USB() {
    this->productID = NIR256_USB_PID;
}

NIR256USB::~NIR256USB() {
}

bool NIR256USB::open() {
    bool retval = false;

    retval = OOIUSBInterface::open();

    if(true == retval) {
        ControlHint *controlHint = new ControlHint();
        SpectrumHint *spectrumHint = new SpectrumHint();
        OOIUSBLegacy2KEndpointMap epMap;

        clearHelpers();

        /* USB 1.1 so use the low-speed transfer strategy. */
        addHelper(spectrumHint, new OOIUSBSpectrumTransferHelper(
                (this->usb), epMap));

        addHelper(controlHint, new OOIUSBControlTransferHelper(
                (this->usb), epMap));
    }

    return retval;
}
