/***************************************************//**
 * @file    FirmwareVersionFeatureAdapter.h
 * @date    March 2020
 * @author  Michele Devetta
 *
 * This is a wrapper that allows
 * access to SeaBreeze FirmwareVersionFeatureInterface instances.
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

#ifndef SEABREEZE_FIRMWAREVERSIONFEATUREADAPTER_H
#define SEABREEZE_FIRMWAREVERSIONFEATUREADAPTER_H

#include "api/seabreezeapi/FeatureAdapterTemplate.h"
#include "vendors/OceanOptics/features/firmware_version/FirmwareVersionFeatureInterface.h"

namespace seabreeze {
    namespace api {

        class FirmwareVersionFeatureAdapter
                : public FeatureAdapterTemplate<FirmwareVersionFeatureInterface> {
        public:
            FirmwareVersionFeatureAdapter(FirmwareVersionFeatureInterface *intf,
                    const FeatureFamily &f,
                    Protocol *p, Bus *b, unsigned short instanceIndex);
            virtual ~FirmwareVersionFeatureAdapter();

            int getFirmwareVersion(int *errorCode, char *buffer, int buffer_length);
            unsigned char getFirmwareVersionMaximumLength(int *errorCode);

        };

    }
}

#endif
