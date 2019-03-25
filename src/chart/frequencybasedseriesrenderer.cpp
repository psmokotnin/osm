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
#include "frequencybasedseriesrenderer.h"
#include "xyplot.h"

using namespace Fftchart;

FrequencyBasedSeriesRenderer::FrequencyBasedSeriesRenderer() :
    m_matrixUniform(0),
    m_minmaxUniform(0),
    m_screenUniform(0),
    m_widthUniform(0)
{

}
void FrequencyBasedSeriesRenderer::setUniforms()
{
    QMatrix4x4 matrix;
    matrix.ortho(0, 1, yMax, yMin, -1, 1);
    matrix.scale(1  / logf(xMax / xMin), 1.0f, 1.0f);
    matrix.translate(-1 * logf(xMin), 0);
    m_program.setUniformValue(m_matrixUniform, matrix);

    m_program.setUniformValue(
        m_minmaxUniform,
        static_cast<GLfloat>(xMin),
        static_cast<GLfloat>(xMax),
        static_cast<GLfloat>(yMin),
        static_cast<GLfloat>(yMax)
    );

    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, 2.0f);
}
void FrequencyBasedSeriesRenderer::iterate(
        unsigned int pointsPerOctave,
        const std::function<void(unsigned int)> &accumulate,
        const std::function<void(float start, float end, unsigned int count)> &collected)
{
    unsigned int count = 0;

    constexpr const float startFrequency = 24000.f / 2048.f;//pow(2, 11);

    float _frequencyFactor = powf(2.f, 1.f / pointsPerOctave);
    float bandStart = startFrequency,
        bandEnd   = bandStart * _frequencyFactor,
        lastBandEnd = bandStart,
        frequency;

    for (unsigned int i = 1; i < m_source->size(); ++i) {
        frequency = m_source->frequency(i);
        if (frequency < bandStart) continue;
        while (frequency > bandEnd) {

            if (count) {
                collected(lastBandEnd, bandEnd, count);
                count = 0;

                //extend current band to the end of the pervious collected
                lastBandEnd = bandEnd;
            }

            bandStart = bandEnd;
            bandEnd   *= _frequencyFactor;
        }
        count ++;
        accumulate(i);
    }
}
void FrequencyBasedSeriesRenderer::iterateForSpline(unsigned int pointsPerOctave,
             float *value, float *coherence,
             const std::function<void (unsigned int)> &accumulate,
             const std::function<void(float f1, float f2, GLfloat *a, GLfloat *c)> &collected
        )
{
    bool bCollected = false;
    unsigned int bCount = 0;
    float splinePoint[4], csplinePoint[4], f[4];
    GLfloat a[4], c[4];

    auto it = [value, coherence, &collected, &splinePoint, &csplinePoint, &f, &a, &c, &bCollected, &bCount]
            (float bandStart, float bandEnd, unsigned int count)
    {
        Q_UNUSED(bandStart)

        *value /= count;
        *coherence /= count;

        if (bCollected) {
            splinePoint[0]  = splinePoint[1];f[0] = f[1];
            splinePoint[1]  = splinePoint[2];f[1] = f[2];
            splinePoint[2]  = splinePoint[3];f[2] = f[3];

            csplinePoint[0]  = csplinePoint[1];
            csplinePoint[1]  = csplinePoint[2];
            csplinePoint[2]  = csplinePoint[3];
        }
        f[bCount] = (bandStart + bandEnd) / 2.f;
        splinePoint[bCount] = *value;
        csplinePoint[bCount] = *coherence;

        if (bCount == 3) {
            a[0] = (     splinePoint[0] + 4 * splinePoint[1] +     splinePoint[2]) / 6;
            a[1] = (-1 * splinePoint[0] +                          splinePoint[2]) / 2;
            a[2] = (     splinePoint[0] - 2 * splinePoint[1] +     splinePoint[2]) / 2;
            a[3] = (-1 * splinePoint[0] + 3 * splinePoint[1] - 3 * splinePoint[2] + splinePoint[3]) / 6;

            c[0] = (     csplinePoint[0] + 4 * csplinePoint[1] +     csplinePoint[2]) / 6;
            c[1] = (-1 * csplinePoint[0] +                           csplinePoint[2]) / 2;
            c[2] = (     csplinePoint[0] - 2 * csplinePoint[1] +     csplinePoint[2]) / 2;
            c[3] = (-1 * csplinePoint[0] + 3 * csplinePoint[1] - 3 * csplinePoint[2] + csplinePoint[3]) / 6;

            bCollected = true;
            collected(f[1], f[2], static_cast<float *>(a), c);
        } else {
            ++bCount;
        }
    };
    iterate(pointsPerOctave, accumulate, it);
}
