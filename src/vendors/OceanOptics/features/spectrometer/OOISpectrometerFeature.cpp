/***************************************************//**
 * @file    OOISpectrometerFeature.cpp
 * @date    February 2009
 * @author  Ocean Optics, Inc.
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
#include "vendors/OceanOptics/protocols/interfaces/SpectrometerProtocolInterface.h"
#include "vendors/OceanOptics/features/eeprom_slots/WavelengthEEPROMSlotFeature.h"
#include "common/exceptions/FeatureProtocolNotFoundException.h"
#include "common/exceptions/FeatureControlException.h"
#include "common/buses/usb/USBTransferHelper.h"
#include "vendors/OceanOptics/protocols/ooi/impls/OOISpectrometerProtocol.h"
#include "vendors/OceanOptics/protocols/ooi/hints/SpectrumHint.h"
#include "common/buses/BusFamilies.h"
#include "api/seabreezeapi/FeatureFamilies.h"
#include "common/Log.h"
#include "vendors/OceanOptics/features/spectrometer/OOISpectrometerFeature.h"
#include "vendors/OceanOptics/protocols/obp/exchanges/OBPIntegrationTimeExchange.h"
#include "vendors/OceanOptics/protocols/obp/exchanges/OBPReadSpectrumWithGainExchange.h"
#include "vendors/OceanOptics/protocols/obp/exchanges/OBPRequestSpectrumExchange.h"
#include "vendors/OceanOptics/protocols/obp/exchanges/OBPTriggerModeExchange.h"
#include "vendors/OceanOptics/protocols/obp/impls/OBPSpectrometerProtocol.h"
#include "vendors/OceanOptics/protocols/obp/exchanges/OBPRequestNumberOfBufferedSpectraWithMetadataExchange.h"

using namespace seabreeze;
using namespace seabreeze::api;
using namespace seabreeze::ooiProtocol;
using namespace std;

#ifdef _WINDOWS
#pragma warning (disable: 4101) // unreferenced local variable
#endif

OOISpectrometerFeature::OOISpectrometerFeature() {

}

OOISpectrometerFeature::~OOISpectrometerFeature() {
    vector<SpectrometerTriggerMode *>::iterator iter;

    for(iter = this->triggerModes.begin(); iter != this->triggerModes.end(); iter++) {
        delete *iter;
    }
}

DoubleVector *OOISpectrometerFeature::getFormattedSpectrum(const Protocol &protocol, const Bus &bus) throw (FeatureException) {

    LOG(__FUNCTION__);
    // logger.debug("starting OOISpectrometerFeature::getFormattedSpectrum");

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    } catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to get a formatted spectrum.");
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureProtocolNotFoundException(error);
    }

    logger.debug("writing requestSpectrum");
    writeRequestFormattedSpectrum(protocol, bus);

    DoubleVector *retval = NULL;

    try {
        logger.debug("reading spectrum");
        retval = spec->readFormattedSpectrum(bus);
    } catch (ProtocolException &pe) {
        string error("Caught protocol exception: ");
        error += pe.what();
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureControlException(error);
    }

    return retval;
}

ByteVector *OOISpectrometerFeature::getUnformattedSpectrum(
        const Protocol &protocol, const Bus &bus) throw (FeatureException) {
    LOG(__FUNCTION__);
    writeRequestUnformattedSpectrum(protocol, bus);
    return readUnformattedSpectrum(protocol, bus);
}

ByteVector *OOISpectrometerFeature::getFastBufferSpectrum(
    const Protocol &protocol, const Bus &bus, unsigned int numberOfSamplesToRetrieve) throw (FeatureException)
{
    LOG(__FUNCTION__);
    writeRequestFastBufferSpectrum(protocol, bus, numberOfSamplesToRetrieve);
    return readFastBufferSpectrum(protocol, bus, numberOfSamplesToRetrieve);
}


// the fast spectrum request reponse pair must be called symetrically. Otherwise the OBP communication will
//  get out of sync
void OOISpectrometerFeature::fastBufferSpectrumRequest(
        const Protocol &protocol, const Bus &bus, unsigned int numberOfSamplesToRetrieve) throw (FeatureException)
{
    LOG(__FUNCTION__);
    writeRequestFastBufferSpectrum(protocol, bus, numberOfSamplesToRetrieve);
}

ByteVector *OOISpectrometerFeature::fastBufferSpectrumResponse(
        const Protocol &protocol, const Bus &bus, unsigned int numberOfSamplesToRetrieve) throw (FeatureException)
{
    LOG(__FUNCTION__);

    return readFastBufferSpectrum(protocol, bus, numberOfSamplesToRetrieve);
}



void OOISpectrometerFeature::writeRequestFormattedSpectrum(const Protocol &protocol,
        const Bus &bus) throw (FeatureException) {

    LOG(__FUNCTION__);
    // logger.debug("starting OOISpectrometerFeature::writeRequestFormattedSpectrum");

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    } catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to get raw spectrum.");
        /* FIXME: previous exception should probably be bundled up into the new exception */
        logger.error(error.c_str());
        throw FeatureProtocolNotFoundException(error);
    }

    try {
        spec->requestFormattedSpectrum(bus);
    } catch (ProtocolException &pe) {
        string error("Caught protocol exception: ");
        error += pe.what();
        /* FIXME: previous exception should probably be bundled up into the new exception */
        logger.error(error.c_str());
        throw FeatureControlException(error);
    }
}

void OOISpectrometerFeature::writeRequestUnformattedSpectrum(const Protocol &protocol,
    const Bus &bus) throw (FeatureException) {

    LOG(__FUNCTION__);
    // logger.debug("starting OOISpectrometerFeature::writeRequestFormattedSpectrum");

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    }
    catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to get an unformatted spectrum.");
        /* FIXME: previous exception should probably be bundled up into the new exception */
        logger.error(error.c_str());
        throw FeatureProtocolNotFoundException(error);
    }

    try {
        spec->requestUnformattedSpectrum(bus);
    }
    catch (ProtocolException &pe) {
        string error("Caught protocol exception: ");
        error += pe.what();
        /* FIXME: previous exception should probably be bundled up into the new exception */
        logger.error(error.c_str());
        throw FeatureControlException(error);
    }
}

void OOISpectrometerFeature::writeRequestFastBufferSpectrum(const Protocol &protocol,
    const Bus &bus, unsigned int numberOfSamplesToRetrieve) throw (FeatureException) {

    LOG(__FUNCTION__);
    // logger.debug("starting OOISpectrometerFeature::writeRequestFormattedSpectrum");

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    }
    catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to get an unformatted spectrum.");
        /* FIXME: previous exception should probably be bundled up into the new exception */
        logger.error(error.c_str());
        throw FeatureProtocolNotFoundException(error);
    }
    
    try {
        spec->requestFastBufferSpectrum(bus, numberOfSamplesToRetrieve);
    }
    catch (ProtocolException &pe) {
        string error("Caught protocol exception: ");
        error += pe.what();
        /* FIXME: previous exception should probably be bundled up into the new exception */
        logger.error(error.c_str());
        throw FeatureControlException(error);
    }
}

ByteVector *OOISpectrometerFeature::readUnformattedSpectrum(const Protocol &protocol,
        const Bus &bus) throw (FeatureException) {
    LOG(__FUNCTION__);
    // logger.debug("starting OOISpectrometerFeature::readUnformattedSpectrum");

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    } catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to get an unformatted spectrum.");
        logger.error(error.c_str());
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureProtocolNotFoundException(error);
    }

    ByteVector *retval;

    try {
        retval = spec->readUnformattedSpectrum(bus);
    } catch (ProtocolException &pe) {
        string error("Caught protocol exception: ");
        error += pe.what();
        logger.error(error.c_str());
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureControlException(error);
    }

    return retval;
}

ByteVector *OOISpectrometerFeature::readFastBufferSpectrum(const Protocol &protocol,
    const Bus &bus, unsigned int numberOfSamplesToRetrieve) throw (FeatureException) {
    LOG(__FUNCTION__);
    // logger.debug("starting OOISpectrometerFeature::readUnformattedSpectrum");

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    }
    catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to get fast buffered spectra.");
        logger.error(error.c_str());
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureProtocolNotFoundException(error);
    }

    ByteVector *retval;

    try {
        retval = spec->readFastBufferSpectrum(bus, numberOfSamplesToRetrieve);
    }
    catch (ProtocolException &pe) {
        string error("Caught protocol exception: ");
        error += pe.what();
        logger.error(error.c_str());
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureControlException(error);
    }

    return retval;
}

vector<double> *OOISpectrometerFeature::getWavelengths(const Protocol &protocol,
        const Bus &bus) throw (FeatureException) {

    WavelengthEEPROMSlotFeature wlFeature(this->numberOfPixels);

    return wlFeature.readWavelengths(protocol, bus);
}

void OOISpectrometerFeature::setIntegrationTimeMicros(const Protocol &protocol,
        const Bus &bus, unsigned long time_usec)
throw (FeatureException, IllegalArgumentException) {

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    } catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to get raw spectrum.");
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureProtocolNotFoundException(error);
    }

    unsigned long iMin = this->integrationTimeMinimum / this->integrationTimeBase;
    unsigned long iMax = this->integrationTimeMaximum / this->integrationTimeBase;
    unsigned long iTime = time_usec / this->integrationTimeBase;

    if(iTime >= iMin && iTime <= iMax) {
        try {
            spec->setIntegrationTimeMicros(bus, time_usec);
        } catch (ProtocolException &pe) {
            string error("Caught protocol exception: ");
            error += pe.what();
            /* FIXME: previous exception should probably be bundled up into the new exception */
            throw FeatureControlException(error);
        }
    } else {
        string error("Specified integration time is out of range.");
        throw IllegalArgumentException(error);
    }
}

void OOISpectrometerFeature::setTriggerMode(const Protocol &protocol,
        const Bus &bus, SpectrometerTriggerMode &mode)
        throw (FeatureException) {
    LOG(__FUNCTION__);

    ProtocolHelper *proto;
    SpectrometerProtocolInterface *spec;

    bool validMode = false;
    vector<SpectrometerTriggerMode *>::iterator iter;

    for(iter = this->triggerModes.begin(); iter != this->triggerModes.end(); iter++) {
        if(mode == *(*iter)) {
            validMode = true;
            break;
        }
    }

    if(false == validMode) {
        string error("Specified trigger mode is not supported.");
        logger.error(error.c_str());
        throw FeatureException(error);
    }

    try {
        proto = lookupProtocolImpl(protocol);
        spec = static_cast<SpectrometerProtocolInterface *>(proto);
    } catch (FeatureProtocolNotFoundException &e) {
        string error("Could not find matching protocol implementation to set trigger mode.");
        logger.error(error.c_str());
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureProtocolNotFoundException(error);
    }

    try {
        spec->setTriggerMode(bus, mode);
    } catch (ProtocolException &pe) {
        string error("Caught protocol exception: ");
        error += pe.what();
        logger.error(error.c_str());
        /* FIXME: previous exception should probably be bundled up into the new exception */
        throw FeatureControlException(error);
    }

}

vector<SpectrometerTriggerMode *> OOISpectrometerFeature::getTriggerModes() const {
    return this->triggerModes;
}

vector<unsigned int> OOISpectrometerFeature::getElectricDarkPixelIndices() const {
    return this->electricDarkPixelIndices;
}

vector<unsigned int> OOISpectrometerFeature::getOpticalDarkPixelIndices() const {
    return this->opticalDarkPixelIndices;
}

vector<unsigned int> OOISpectrometerFeature::getActivePixelIndices() const {
    return this->activePixelIndices;
}

long OOISpectrometerFeature::getIntegrationTimeMinimum() const {
    return this->integrationTimeMinimum;
}

long OOISpectrometerFeature::getIntegrationTimeMaximum() const {
    return this->integrationTimeMaximum;
}

long OOISpectrometerFeature::getIntegrationTimeIncrement() const {
    return this->integrationTimeIncrement;
}

unsigned short OOISpectrometerFeature::getNumberOfPixels() const {
    return this->numberOfPixels;
}

int OOISpectrometerFeature::getMaximumIntensity() const {
    return this->maxIntensity;
}

void OOISpectrometerFeature::setTimeout(const Bus &bus, unsigned int timeout) throw (FeatureException) {
//     BusFamily bf = bus.getBusFamily();
//     BusFamilies families;
//     if(!bf.equals(families.USB))
//         throw FeatureException("");

    // Create a hints vector
    vector<ProtocolHint *> hints;
    hints.push_back(new SpectrumHint());

    // Get transfer helper for spectra (SpectrumHint)
    TransferHelper* helper;
    helper = bus.getHelper(hints);

    // Cleanup hints
    delete hints[0];
    hints.clear();

    if(NULL == helper) {
        throw FeatureException("Failed to get spectrum transfer helper");
    }

    // Dynamic cast to USBTransferHelper to check if it's a USB spectrometer
    USBTransferHelper* usb_helper = dynamic_cast<USBTransferHelper*>(helper);
    if(NULL == usb_helper) {
        throw FeatureException("This is supported only on USB spectrometers");
    }

    // Set timeout on helper
    usb_helper->setTimeout(timeout);
}

FeatureFamily OOISpectrometerFeature::getFeatureFamily() {
    FeatureFamilies families;

    return families.SPECTROMETER;
}

