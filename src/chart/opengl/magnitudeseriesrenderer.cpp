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
#include "magnitudeseriesrenderer.h"
#include "../magnitudeplot.h"
#include <cstring>
#include "common/notifier.h"
using namespace Chart;

MagnitudeSeriesRenderer::MagnitudeSeriesRenderer() : FrequencyBasedSeriesRenderer(),
    m_pointsPerOctave(0), m_coherenceThreshold(0), m_coherence(false), m_mode(MagnitudePlot::Mode::dB)
{

}
void MagnitudeSeriesRenderer::init()
{
    if (m_openGL33CoreFunctions) {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/magnitude.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/magnitude.geom");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/magnitude.frag");
    } else {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl2/logx.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl2/color.frag");
    }
    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("MagnitudeSeriesRenderer", m_program.log());
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
void MagnitudeSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<MagnitudePlot *>(m_item ? m_item->parent() : nullptr)) {
        m_pointsPerOctave    = plot->pointsPerOctave();
        m_coherence          = plot->coherence();
        m_invert             = plot->invert();
        m_mode               = plot->mode();
        m_sensor             = plot->sensor();
        m_coherenceThreshold = plot->coherenceThreshold();
    }
}
void MagnitudeSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->frequencyDomainSize())
        return;

    unsigned int maxBufferSize = m_pointsPerOctave * 12 * (m_openGL33CoreFunctions ? 12 : PPO_BUFFER_MUL), i = 0,
                 verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float value = 0.f, coherence = 0.f;

    float xadd, xmul;
    xadd = -1.0f * logf(m_xMin);
    xmul = m_width / logf(m_xMax / m_xMin);

    auto accumulate = [this, &coherence, &value] (const unsigned int &i) {
        coherence += m_source->coherence(i);

        switch (m_mode) {
        case MagnitudePlot::Mode::Linear:
            value += std::abs(std::pow(m_source->magnitudeRaw(i), m_invert ? -1 : 1));
            break;

        case MagnitudePlot::Mode::Impedance:
            value += std::abs(std::pow(m_source->magnitudeRaw(i), m_invert ? -1 : 1)) * m_sensor - m_sensor;
            break;

        case MagnitudePlot::Mode::dB:
            value += (m_invert ? -1 : 1) * m_source->magnitude(i);
            break;
        }
    };

    auto collected = [ &, this] (const float & f1, const float & f2, const float * ac, const float * c) {

        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;

        if (m_openGL33CoreFunctions) {
            if (i + 12 > maxBufferSize) {
                return;
            }
            m_vertices[i + 0] = f1;
            m_vertices[i + 1] = f2;
            m_vertices[i + 2] = fx1;
            m_vertices[i + 3] = fx2;
            std::memcpy(m_vertices.data() + i + 4, ac, 4 * 4);
            std::memcpy(m_vertices.data() + i + 8,  c, 4 * 4);
            verticiesCount ++;
            i += 12;
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

                addLineSegment(i, verticiesCount,
                               x1, v1,
                               x2, v2,
                               c1, c2);
            }
        }
        value = 0.f;
        coherence = 0.f;
    };

    iterateForSpline<float, float>(m_pointsPerOctave, &value, &coherence, accumulate, collected);

    setUniforms();
    if (m_openGL33CoreFunctions) {
        m_program.setUniformValue(m_coherenceThresholdU, m_coherenceThreshold);
        m_program.setUniformValue(m_coherenceAlpha, m_coherence);

        if (m_refreshBuffers) {
            m_openGLFunctions->glGenBuffers(1, &m_vertexBufferId);
            m_openGL33CoreFunctions->glGenVertexArrays(1, &m_vertexArrayId);
        }

        m_openGL33CoreFunctions->glBindVertexArray(m_vertexArrayId);
        m_openGLFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

        if (m_refreshBuffers) {
            m_openGLFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
            m_openGLFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat),
                                                     reinterpret_cast<const void *>(0));
            m_openGLFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat),
                                                     reinterpret_cast<const void *>(2 * sizeof(GLfloat)));
            m_openGLFunctions->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat),
                                                     reinterpret_cast<const void *>(4 * sizeof(GLfloat)));
            m_openGLFunctions->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat),
                                                     reinterpret_cast<const void *>(8 * sizeof(GLfloat)));
        }
        m_openGLFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

        m_openGLFunctions->glEnableVertexAttribArray(0);
        m_openGLFunctions->glEnableVertexAttribArray(1);
        m_openGLFunctions->glEnableVertexAttribArray(2);
        m_openGLFunctions->glEnableVertexAttribArray(3);
        m_openGLFunctions->glDrawArrays(GL_POINTS, 0, verticiesCount);
        m_openGLFunctions->glDisableVertexAttribArray(3);
        m_openGLFunctions->glDisableVertexAttribArray(2);
        m_openGLFunctions->glDisableVertexAttribArray(1);
        m_openGLFunctions->glDisableVertexAttribArray(0);
        m_refreshBuffers = false;
    } else {
        drawOpenGL2(verticiesCount);
    }

}
