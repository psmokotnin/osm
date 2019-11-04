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
#include <cmath>
#include "groupdelayseriesrenderer.h"
#include "groupdelayplot.h"

using namespace Fftchart;

GroupDelaySeriesRenderer::GroupDelaySeriesRenderer() : m_pointsPerOctave(0), m_coherence(false)
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/logx.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/magnitude.frag");
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
    m_coherenceSpline = m_program.uniformLocation("coherenceSpline");
}
void GroupDelaySeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<GroupDelayPlot*>(m_item->parent())) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_coherence = plot->coherence();
    }
}
void GroupDelaySeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    GLfloat vertices[8];
    float value(0);
    float coherence = 0.f;

    setUniforms();
    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));
    openGLFunctions->glEnableVertexAttribArray(0);

    float xadd, xmul;
    xadd = -1.0f * logf(xMin);
    xmul = m_width / logf(xMax / xMin);

    constexpr float
            d = 0.85f,
            a = (1 - d*d),
            b = d*d / a;
    auto accumulate = [&value, &coherence, m_source = m_source, m_coherence = m_coherence] (unsigned int i)
    {
        if (i > 0) {
            complex a = m_source->phase(i), b = m_source->phase(i - 1);
            float ab = (a.real * b.real + a.imag * b.imag) / (a.abs() * b.abs());
            while(std::abs(ab) > 1.f) {
                ab = std::copysign(1.f, ab);
            }
            float dP = std::acos(ab) /
                    (m_source->frequency(i) - m_source->frequency(i - 1));
            value -= dP;
        }
        coherence += (m_coherence ? powf(m_source->coherence(i), 2) / a - b : 1.f);
    };
    auto collected = [m_program = &m_program, openGLFunctions = openGLFunctions, &vertices,
                    m_splineA = m_splineA,
                    &value, &coherence,
                    m_frequency1 = m_frequency1, m_frequency2 = m_frequency2,
                    xadd, xmul, yMin = yMin, yMax = yMax, m_coherenceSpline = m_coherenceSpline]
            (float f1, float f2, float ac[4], float c[4])
    {
        vertices[0] = f1;
        vertices[1] = yMin;
        vertices[2] = f1;
        vertices[3] = yMax;
        vertices[4] = f2;
        vertices[5] = yMax;
        vertices[6] = f2;
        vertices[7] = yMin;

        m_program->setUniformValueArray(m_splineA, static_cast<GLfloat *>(ac), 1, 4);
        m_program->setUniformValueArray(m_coherenceSpline, c, 1, 4);
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;
        m_program->setUniformValue(m_frequency1, fx1);
        m_program->setUniformValue(m_frequency2, fx2);
        openGLFunctions->glDrawArrays(GL_QUADS, 0, 4);

        value = 0.0f;
        coherence = 0.f;
    };

    iterateForSpline<float>(m_pointsPerOctave, &value, &coherence, accumulate, collected);

    openGLFunctions->glDisableVertexAttribArray(0);
}
