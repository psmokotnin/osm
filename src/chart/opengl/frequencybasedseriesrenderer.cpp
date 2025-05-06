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
#include "frequencybasedseriesrenderer.h"
#include "../xyplot.h"

using namespace Chart;

FrequencyBasedSeriesRenderer::FrequencyBasedSeriesRenderer() : XYSeriesRenderer(true, false),
    m_minmaxUniform(0),
    m_screenUniform(0),
    m_widthUniform(0)
{

}
void FrequencyBasedSeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMax, m_yMin, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), 0);
}

void FrequencyBasedSeriesRenderer::setUniforms()
{
    m_program.setUniformValue(m_matrixUniform, m_matrix);
    m_program.setUniformValue(
        m_minmaxUniform,
        static_cast<GLfloat>(m_xMin),
        static_cast<GLfloat>(m_xMax),
        static_cast<GLfloat>(m_yMin),
        static_cast<GLfloat>(m_yMax)
    );

    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, m_weight * m_retinaScale);
}

const Shared::Source &FrequencyBasedSeriesRenderer::source() const
{
    return m_source;
}
