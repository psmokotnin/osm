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
#include "common/notifier.h"
using namespace Chart;

PhaseSeriesRenderer::PhaseSeriesRenderer() : FrequencyBasedSeriesRenderer(),
    m_coherenceThresholdU(0), m_coherenceAlpha(0),
    m_pointsPerOctave(0), m_rotate(0),
    m_coherenceThreshold(0), m_coherence(false)
{
}
void PhaseSeriesRenderer::init()
{
    if (m_openGL33CoreFunctions) {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/phase.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/phase.geom");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/phase.frag");
    } else {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl2/logx.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl2/color.frag");
    }
    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("PhaseSeriesRenderer", m_program.log());
    }

    if (m_openGL33CoreFunctions) {
        m_widthUniform  = m_program.uniformLocation("width");
        m_colorUniform  = m_program.uniformLocation("m_color");
        m_matrixUniform = m_program.uniformLocation("matrix");
        m_minmaxUniform = m_program.uniformLocation("minmax");
        m_screenUniform = m_program.uniformLocation("screen");
        m_coherenceThresholdU = m_program.uniformLocation("coherenceThreshold");
        m_coherenceAlpha      = m_program.uniformLocation("coherenceAlpha");
    } else {
        m_positionAttribute = m_program.attributeLocation("position");
        m_colorUniform  = m_program.attributeLocation("color");
        m_matrixUniform = m_program.uniformLocation("matrix");
    }
}
void PhaseSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *phasePlot = dynamic_cast<PhasePlot *>(plot())) {
        m_pointsPerOctave = phasePlot->pointsPerOctave();
        m_coherence = phasePlot->coherence();
        constexpr float pk = static_cast<float>(-M_PI / 180.0);
        m_rotate = phasePlot->rotate() *  pk;
        m_coherenceThreshold = phasePlot->coherenceThreshold();
    }
}
void PhaseSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->frequencyDomainSize())
        return;

    //max octave count: 11
    unsigned int maxBufferSize = m_pointsPerOctave * 12 * (m_openGL33CoreFunctions ? 16 : PPO_BUFFER_MUL), i = 0,
                 verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    Complex value(0);
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
    auto collected = [ &, this] (const float & f1, const float & f2, const Complex ac[4], const float c[4]) {
        if (i + 16 > maxBufferSize) {
            return;
        }
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;

        if (m_openGL33CoreFunctions) {
            if (i + 16 > maxBufferSize) {
                return;
            }
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
        } else {
            auto points = std::min(MAX_LINE_SPLITF, std::abs(std::round(fx2 - fx1)));
            float dt = 1.f / points;

            for (float t = 0; t < 1.0;) {
                auto x1 = f1 * std::pow(f2 / f1, t);
                auto v1 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;
                auto c1 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);
                t += dt;
                auto x2 = f1 * std::pow(f2 / f1, t);
                auto v2 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;
                auto c2 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

                float alpha = std::atan2(v1.imag, v1.real);
                float beta = std::atan2(v2.imag, v2.real);
                if (std::abs(alpha - beta) > M_PI) {
                    if (alpha > 0) {
                        alpha -= 2 * M_PI;
                    } else {
                        alpha += 2 * M_PI;
                    }
                }

                addLineSegment(i, verticiesCount,
                               x1, alpha,
                               x2, beta,
                               c1, c2);
            }
        }

        value = 0.f;
        coherence = 0.f;
    };

    iterateForSpline<Complex, Complex>(m_pointsPerOctave, &value, &coherence, accumulate, collected, beforeSpline);

    setUniforms();
    if (m_openGL33CoreFunctions) {
        m_program.setUniformValue(m_coherenceThresholdU, m_coherenceThreshold);
        m_program.setUniformValue(m_coherenceAlpha, m_coherence);

        if (m_refreshBuffers) {
            m_openGL33CoreFunctions->glGenBuffers(1, &m_vertexBufferId);
            m_openGL33CoreFunctions->glGenVertexArrays(1, &m_vertexArrayId);
        }

        m_openGL33CoreFunctions->glBindVertexArray(m_vertexArrayId);
        m_openGL33CoreFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

        if (m_refreshBuffers) {
            m_openGL33CoreFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
            m_openGL33CoreFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(0));
            m_openGL33CoreFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(2 * sizeof(GLfloat)));
            m_openGL33CoreFunctions->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(4 * sizeof(GLfloat)));
            m_openGL33CoreFunctions->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(8 * sizeof(GLfloat)));
            m_openGL33CoreFunctions->glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(12 * sizeof(GLfloat)));
        }
        m_openGL33CoreFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

        m_openGL33CoreFunctions->glEnableVertexAttribArray(0);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(1);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(2);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(3);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(4);
        m_openGL33CoreFunctions->glDrawArrays(GL_POINTS, 0, verticiesCount);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(4);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(3);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(2);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(1);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(0);

        m_refreshBuffers = false;
    } else {
        m_openGLFunctions->glLineWidth(m_weight * m_retinaScale);

        m_openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_positionAttribute), 2,
                                                 GL_FLOAT, GL_FALSE, LINE_VERTEX_SIZE * sizeof(GLfloat),
                                                 static_cast<const void *>(m_vertices.data()));
        m_openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_colorUniform), 4,
                                                 GL_FLOAT, GL_FALSE, LINE_VERTEX_SIZE * sizeof(GLfloat),
                                                 static_cast<const void *>(&m_vertices[2]));

        m_openGLFunctions->glEnableVertexAttribArray(0);
        m_openGLFunctions->glEnableVertexAttribArray(1);
        m_openGLFunctions->glDrawArrays(GL_LINES, 0, verticiesCount);
        m_openGLFunctions->glDisableVertexAttribArray(1);
        m_openGLFunctions->glDisableVertexAttribArray(0);
    }
}
