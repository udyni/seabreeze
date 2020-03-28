/***************************************************//**
 * @file    FPGAFirmwareVersionFeature.cpp
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
#include "vendors/OceanOptics/features/fpga_register/FPGAFirmwareVersionFeature.h"
#include "vendors/OceanOptics/protocols/interfaces/FPGARegisterProtocolInterface.h"
#include "vendors/OceanOptics/protocols/ooi/impls/FPGARegisterProtocol.h"
#include "vendors/OceanOptics/protocols/ooi/constants/FPGARegisterCodes.h"
#include "common/exceptions/FeatureProtocolNotFoundException.h"
#include "common/exceptions/FeatureControlException.h"
#include "api/seabreezeapi/FeatureFamilies.h"
#include <sstream>
#include <string>
#include <iomanip>

using namespace seabreeze;
using namespace seabreeze::ooiProtocol;
using namespace seabreeze::api;
using namespace std;


FPGAFirmwareVersionFeature::FPGAFirmwareVersionFeature() {

}

FPGAFirmwareVersionFeature::~FPGAFirmwareVersionFeature() {

}

std::string* FPGAFirmwareVersionFeature::readFirmwareVersion(const Protocol &protocol, const Bus &bus) throw (FeatureException) {
    FPGARegisterCodes::Base codes;
    FPGARegisterProtocol proto;
    FPGARegisterProtocolInterface *fpga = &proto;
    std::string* retval;

    try {
        // firmware version appears to be packed into two bytes as follows:
        //
        //    MSB       LSB
        //  01234567 01234567
        //  \__/\_______/\__/
        // major  minor  patch
        unsigned int version = fpga->readRegister(bus, codes.FIRMWARE_VERSION) & 0x00FFFF;
        unsigned char major = (version >> 12) & 0x0F;
        unsigned char minor = (version >> 4) & 0xFF;
        unsigned char patch = (version) & 0x0F;

        stringstream firmware;
        firmware << int(major) << "." << setfill('0') << setw(2) << int(minor) << "." << int(patch);
        retval = new std::string(firmware.str());
        return retval;

    } catch (ProtocolException &pe) {
        string error = string("Caught protocol exception: ") + pe.what();
        throw FeatureControlException(error);
    }
}

unsigned char FPGAFirmwareVersionFeature::readFirmwareVersionMaximumLength(const Protocol &protocol, const Bus &bus) throw (FeatureException) {
    std::string* firmware = readFirmwareVersion(protocol, bus);
    unsigned char len = (unsigned char)firmware->size();
    delete firmware;
    return len;
}


FeatureFamily FPGAFirmwareVersionFeature::getFeatureFamily() {
    FeatureFamilies families;
    return families.FIRMWARE_VERSION;
}
