/***************************************************//**
 * @file    NIR512SpectrometerFeature.h
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

#ifndef NIR512SPECTROMETERFEATURE_H
#define NIR512SPECTROMETERFEATURE_H

#include "vendors/OceanOptics/features/spectrometer/OOISpectrometerFeature.h"

namespace seabreeze {

    class NIR512SpectrometerFeature : public OOISpectrometerFeature {
    public:
        NIR512SpectrometerFeature();
        virtual ~NIR512SpectrometerFeature();

    protected:
        static const long INTEGRATION_TIME_MINIMUM;
        static const long INTEGRATION_TIME_MAXIMUM;
        static const long INTEGRATION_TIME_INCREMENT;
        static const long INTEGRATION_TIME_BASE;
    };

}

#endif /* NIR512SPECTROMETERFEATURE_H */
