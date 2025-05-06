/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FREQUENCYBASEDSERIESRENDERER_H
#define FREQUENCYBASEDSERIESRENDERER_H

#include "xyseriesrenderer.h"
#include "../frequencybasedserieshelper.h"

namespace Chart {

class FrequencyBasedSeriesRenderer : public XYSeriesRenderer, public FrequencyBasedSeriesHelper
{

protected:
    constexpr const static float LEVEL_NORMALIZATION = 0;

    virtual void updateMatrix() override;
    void setUniforms();
    const Shared::Source &source() const override;

public:
    explicit FrequencyBasedSeriesRenderer();

protected:
    int m_minmaxUniform,
        m_screenUniform,
        m_widthUniform;
};
}
#endif // FREQUENCYBASEDSERIESRENDERER_H
