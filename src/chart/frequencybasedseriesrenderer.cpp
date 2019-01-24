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
    SeriesRenderer(),
    m_matrixUniform(0),
    m_minmaxUniform(0),
    m_screenUniform(0),
    m_widthUniform(0)
{

}
void FrequencyBasedSeriesRenderer::setUniforms()
{
    XYPlot *plot = static_cast<XYPlot*>(m_item->parent());

    QMatrix4x4 matrix;
    matrix.ortho(0, 1, plot->yAxis()->max(), plot->yAxis()->min(), -1, 1);
    matrix.scale(1  / logf(plot->xAxis()->max() / plot->xAxis()->min()), 1.0f, 1.0f);
    matrix.translate(-1 * logf(plot->xAxis()->min()), 0);
    m_program.setUniformValue(m_matrixUniform, matrix);

    m_program.setUniformValue(
        m_minmaxUniform,
        static_cast<GLfloat>(plot->xAxis()->min()),
        static_cast<GLfloat>(plot->xAxis()->max()),
        static_cast<GLfloat>(plot->yAxis()->min()),
        static_cast<GLfloat>(plot->yAxis()->max())
    );

    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, 2.0f);
}
void FrequencyBasedSeriesRenderer::iterate(
        unsigned int pointsPerOctave,
        std::function<void(unsigned int)> accumulate,
        std::function<void(float start, float end, unsigned int count)> collected)
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
             float *value,
             std::function<void (unsigned int)> accumulate,
             std::function<void(float f1, float f2, GLfloat *a)> collected
        )
{
    bool bCollected = false;
    unsigned int bCount = 0;
    float splinePoint[4], f[4];
    GLfloat a[4];

    auto it = [value, &collected, &splinePoint, &f, &a, &bCollected, &bCount]
            (float bandStart, float bandEnd, unsigned int count)
    {
        Q_UNUSED(bandStart)

        *value /= count;

        if (bCollected) {
            splinePoint[0]  = splinePoint[1];f[0] = f[1];
            splinePoint[1]  = splinePoint[2];f[1] = f[2];
            splinePoint[2]  = splinePoint[3];f[2] = f[3];
        }
        f[bCount] = (bandStart + bandEnd) / 2.f;
        splinePoint[bCount] = *value;

        if (bCount == 3) {
            a[0] = (     splinePoint[0] + 4 * splinePoint[1] +     splinePoint[2]) / 6;
            a[1] = (-1 * splinePoint[0] +                          splinePoint[2]) / 2;
            a[2] = (     splinePoint[0] - 2 * splinePoint[1] +     splinePoint[2]) / 2;
            a[3] = (-1 * splinePoint[0] + 3 * splinePoint[1] - 3 * splinePoint[2] + splinePoint[3]) / 6;
            bCollected = true;
            collected(f[1], f[2], a);
        } else {
            ++bCount;
        }
    };
    iterate(pointsPerOctave, accumulate, it);
}
