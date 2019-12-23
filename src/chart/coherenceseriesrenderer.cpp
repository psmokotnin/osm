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
#include "coherenceseriesrenderer.h"

using namespace Fftchart;

CoherenceSeriesRenderer::CoherenceSeriesRenderer() : m_pointsPerOctave(24)
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/logx.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/coherence.frag");
    m_program.link();
    m_posAttr = m_program.attributeLocation("posAttr");

    m_splineA       = m_program.uniformLocation("splineA");
    m_frequency1    = m_program.uniformLocation("frequency1");
    m_frequency2    = m_program.uniformLocation("frequency2");
    m_widthUniform  = m_program.uniformLocation("width");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
    m_minmaxUniform = m_program.uniformLocation("minmax");
    m_screenUniform = m_program.uniformLocation("screen");
}
void CoherenceSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<CoherencePlot*>(m_item->parent())) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_type = plot->type();
    }
}
void CoherenceSeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    GLfloat vertices[8];
    float value = 0.f, coherence = 1.f;

    setUniforms();
    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));
    openGLFunctions->glEnableVertexAttribArray(0);

    float xadd, xmul;
    xadd = -1.0f * logf(xMin);
    xmul = m_width / logf(xMax / xMin);

    /*
     * Draw quad for each band from min to max (full height)
     * pass spline data to shaders
     * fragment shader draws spline function
     */
    auto accumulate = [m_source = m_source, &value, m_type = m_type] (unsigned int i)
    {
        value += (m_type == CoherencePlot::Type::SQUARED ? powf(m_source->coherence(i), 2) : m_source->coherence(i));
    };
    auto collected = [m_program = &m_program, openGLFunctions = openGLFunctions, &vertices, &value,
            m_splineA = m_splineA, m_frequency1 = m_frequency1, m_frequency2 = m_frequency2,
            xadd, xmul, yMin = yMin, yMax = yMax]
            (float f1, float f2, float *ac, float *c)
    {
        Q_UNUSED(c)
        vertices[0] = f1;
        vertices[1] = yMin;
        vertices[2] = f1;
        vertices[3] = yMax;
        vertices[4] = f2;
        vertices[5] = yMax;
        vertices[6] = f2;
        vertices[7] = yMin;

        m_program->setUniformValueArray(m_splineA, static_cast<GLfloat *>(ac), 1, 4);
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;
        m_program->setUniformValue(m_frequency1, fx1);
        m_program->setUniformValue(m_frequency2, fx2);
        openGLFunctions->glDrawArrays(GL_QUADS, 0, 4);

        value = 0.0f;
    };

    iterateForSpline<float, float>(m_pointsPerOctave, &value, &coherence, accumulate, collected);
    openGLFunctions->glDisableVertexAttribArray(0);
}
