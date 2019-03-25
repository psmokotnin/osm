/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
namespace Fftchart {

class FrequencyBasedSeriesRenderer : public XYSeriesRenderer
{

protected:
    void setUniforms();
    void iterate(unsigned int pointsPerOctave,
                const std::function<void(unsigned int)> &accumulate,
                const std::function<void(float start, float end, unsigned int count)> &collected
            );

    void iterateForSpline(unsigned int pointsPerOctave,
                float *value, float *coherence,
                const std::function<void (unsigned int)> &accumulate,
                const std::function<void(float f1, float f2, GLfloat *a, GLfloat *c)> &collected
            );

public:
    explicit FrequencyBasedSeriesRenderer();
    virtual void renderSeries() = 0;

protected:
    int m_matrixUniform,
        m_minmaxUniform,
        m_screenUniform,
        m_widthUniform;
};
}
#endif // FREQUENCYBASEDSERIESRENDERER_H
