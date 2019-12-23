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
#include "phaseseriesrenderer.h"
#include "phaseplot.h"

using namespace Fftchart;

PhaseSeriesRenderer::PhaseSeriesRenderer() : m_pointsPerOctave(0), m_rotate(0), m_coherence(false)
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/logx.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/phase.frag");
    m_program.link();
    m_posAttr = m_program.attributeLocation("posAttr");

    m_splineRe       = m_program.uniformLocation("splineRe");
    m_splineIm       = m_program.uniformLocation("splineIm");
    m_frequency1    = m_program.uniformLocation("frequency1");
    m_frequency2    = m_program.uniformLocation("frequency2");

    m_widthUniform  = m_program.uniformLocation("width");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
    m_minmaxUniform = m_program.uniformLocation("minmax");
    m_screenUniform = m_program.uniformLocation("screen");
    m_coherenceSpline = m_program.uniformLocation("coherenceSpline");
}
void PhaseSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<PhasePlot*>(m_item->parent())) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_coherence = plot->coherence();
        constexpr float pk = static_cast<float>(-M_PI / 180.0);
        m_rotate = plot->rotate() *  pk;
    }
}
void PhaseSeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    GLfloat vertices[8], re[4], im[4];
    complex value(0);
    float coherence = 0.f;
    constexpr float F_PI = static_cast<float>(M_PI);

    setUniforms();
    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));
    openGLFunctions->glEnableVertexAttribArray(0);

    float xadd, xmul;
    xadd = -1.0f * logf(xMin);
    xmul = m_width / logf(xMax / xMin);

    /*
     * Draw quad for each band from -PI to +PI (full height)
     * pass spline data to shaders
     * fragment shader draws phase spline function
     */
    constexpr float thershold = 0.85f;
    auto accumulate = [&value, &coherence, m_rotate = m_rotate, m_source = m_source, m_coherence = m_coherence] (unsigned int i)
    {
        value += m_source->phase(i).rotate(m_rotate);
        coherence += (m_coherence ?
                     (m_source->coherence(i) > thershold ? m_source->coherence(i) : 0.f) :
                     1.f);
    };
    auto collected = [m_program = &m_program, openGLFunctions = openGLFunctions, &vertices,
                    m_splineRe = m_splineRe, m_splineIm = m_splineIm,
                    &value, &coherence, &re, &im,
                    m_frequency1 = m_frequency1, m_frequency2 = m_frequency2,
                    xadd, xmul, m_coherenceSpline = m_coherenceSpline]
            (float f1, float f2, complex ac[4], float c[4])
    {
        vertices[0] =  f1;
        vertices[1] = -F_PI;
        vertices[2] =  f1;
        vertices[3] =  F_PI;
        vertices[4] =  f2;
        vertices[5] =  F_PI;
        vertices[6] =  f2;
        vertices[7] = -F_PI;

        re[0] = ac[0].real;im[0] = ac[0].imag;
        re[1] = ac[1].real;im[1] = ac[1].imag;
        re[2] = ac[2].real;im[2] = ac[2].imag;
        re[3] = ac[3].real;im[3] = ac[3].imag;

        m_program->setUniformValueArray(m_splineRe, re, 1, 4);
        m_program->setUniformValueArray(m_splineIm, im, 1, 4);
        m_program->setUniformValueArray(m_coherenceSpline, c, 1, 4);
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;
        m_program->setUniformValue(m_frequency1, fx1);
        m_program->setUniformValue(m_frequency2, fx2);
        openGLFunctions->glDrawArrays(GL_QUADS, 0, 4);

        value.real = value.imag = 0.0f;
        coherence = 0.f;
    };

    iterateForSpline<complex, complex>(m_pointsPerOctave, &value, &coherence, accumulate, collected);

    openGLFunctions->glDisableVertexAttribArray(0);
}
