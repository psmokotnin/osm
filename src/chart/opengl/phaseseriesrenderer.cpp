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
#include <cmath>
#include "phaseseriesrenderer.h"
#include "../phaseplot.h"
#include <QtMath>
#include <cstring>

using namespace chart;

PhaseSeriesRenderer::PhaseSeriesRenderer() : FrequencyBasedSeriesRenderer(),
    m_pointsPerOctave(0), m_rotate(0),
    m_coherenceThreshold(0), m_coherence(false)
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/phase.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/phase.geom");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/phase.frag");
    m_program.link();

    m_widthUniform  = m_program.uniformLocation("width");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
    m_minmaxUniform = m_program.uniformLocation("minmax");
    m_screenUniform = m_program.uniformLocation("screen");
    m_coherenceThresholdU = m_program.uniformLocation("coherenceThreshold");
    m_coherenceAlpha      = m_program.uniformLocation("coherenceAlpha");
}
void PhaseSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<PhasePlot *>(m_item->parent())) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_coherence = plot->coherence();
        constexpr float pk = static_cast<float>(-M_PI / 180.0);
        m_rotate = plot->rotate() *  pk;
        m_coherenceThreshold = plot->coherenceThreshold();
    }
}
void PhaseSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->size())
        return;

    //max octave count: 11
    unsigned int maxBufferSize = m_pointsPerOctave * 12 * 16, i = 0, verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    complex value(0);
    float coherence = 0.f;

    float xadd, xmul;
    xadd = -1.0f * logf(m_xMin);
    xmul = m_width / logf(m_xMax / m_xMin);

    auto accumulate = [&value, &coherence, this] (const unsigned int &i) {
        value += m_source->phase(i);
        coherence += m_source->coherence(i);
    };
    auto beforeSpline = [this] (const auto * value, auto, const auto & count) {
        return value->rotate(m_rotate) / count;
    };
    auto collected = [ &, this] (const float & f1, const float & f2, const complex ac[4], const float c[4]) {
        if (i > maxBufferSize) {
            qCritical("out of range");
            return;
        }
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;

        m_vertices[i + 0] = f1;
        m_vertices[i + 1] = f2;
        m_vertices[i + 2] = fx1;
        m_vertices[i + 3] = fx2;

        m_vertices[i +  4] = ac[0].real;
        m_vertices[i +  5] = ac[1].real;
        m_vertices[i +  6] = ac[2].real;
        m_vertices[i +  7] = ac[3].real;

        m_vertices[i +  8] = ac[0].imag;
        m_vertices[i +  9] = ac[1].imag;
        m_vertices[i + 10] = ac[2].imag;
        m_vertices[i + 11] = ac[3].imag;
        std::memcpy(m_vertices.data() + i + 12,  c, 4 * 4);
        verticiesCount ++;
        i += 16;

        value = 0.f;
        coherence = 0.f;
    };

    iterateForSpline<complex, complex>(m_pointsPerOctave, &value, &coherence, accumulate, collected, beforeSpline);

    setUniforms();
    m_program.setUniformValue(m_coherenceThresholdU, m_coherenceThreshold);
    m_program.setUniformValue(m_coherenceAlpha, m_coherence);

    if (m_refreshBuffers) {
        m_openGLFunctions->glGenBuffers(1, &m_vertexBufferId);
        m_openGLFunctions->glGenVertexArrays(1, &m_vertexArrayId);
    }

    m_openGLFunctions->glBindVertexArray(m_vertexArrayId);
    m_openGLFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

    if (m_refreshBuffers) {
        m_openGLFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
        m_openGLFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(0));
        m_openGLFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(2 * sizeof(GLfloat)));
        m_openGLFunctions->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(4 * sizeof(GLfloat)));
        m_openGLFunctions->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(8 * sizeof(GLfloat)));
        m_openGLFunctions->glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(12 * sizeof(GLfloat)));
    }
    m_openGLFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

    m_openGLFunctions->glEnableVertexAttribArray(0);
    m_openGLFunctions->glEnableVertexAttribArray(1);
    m_openGLFunctions->glEnableVertexAttribArray(2);
    m_openGLFunctions->glEnableVertexAttribArray(3);
    m_openGLFunctions->glEnableVertexAttribArray(4);
    m_openGLFunctions->glDrawArrays(GL_POINTS, 0, verticiesCount);
    m_openGLFunctions->glDisableVertexAttribArray(4);
    m_openGLFunctions->glDisableVertexAttribArray(3);
    m_openGLFunctions->glDisableVertexAttribArray(2);
    m_openGLFunctions->glDisableVertexAttribArray(1);
    m_openGLFunctions->glDisableVertexAttribArray(0);

    m_refreshBuffers = false;
}
