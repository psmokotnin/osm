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
#include "nyquistseriesrenderer.h"
#include "common/notifier.h"
#include "../nyquistplot.h"
#include <cstring>

namespace Chart {

NyquistSeriesRenderer::NyquistSeriesRenderer() : XYSeriesRenderer(false, false),
    m_pointsPerOctave(0),
    m_coherenceThreshold(0), m_coherence(false)
{
}

void NyquistSeriesRenderer::init()
{
    if (m_openGL33CoreFunctions) {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/nyquist.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/nyquist.geom");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/nyquist.frag");
    } else {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl2/point.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl2/color.frag");
    }
    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("NyquistSeriesRenderer", m_program.log());
    }
    if (m_openGL33CoreFunctions) {
        m_widthUniform  = m_program.uniformLocation("width");
        m_colorUniform  = m_program.uniformLocation("m_color");
        m_matrixUniform = m_program.uniformLocation("matrix");
        m_screenUniform = m_program.uniformLocation("screen");
        m_coherenceThresholdU = m_program.uniformLocation("coherenceThreshold");
        m_coherenceAlpha      = m_program.uniformLocation("coherenceAlpha");
    } else {
        m_positionAttribute = m_program.attributeLocation("position");
        m_colorUniform  = m_program.attributeLocation("color");
        m_matrixUniform = m_program.uniformLocation("matrix");
    }
}

void NyquistSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->frequencyDomainSize())
        return;

    //max octave count: 11
    unsigned int maxBufferSize = m_pointsPerOctave * 12 * (m_openGL33CoreFunctions ? 12 : PPO_BUFFER_MUL), i = 0,
                 verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    NyquistPlot::SplineValue value(0, 0);
    float coherence = 0.f;

    auto accumulate = [&value, &coherence, this] (const unsigned int &i) {
        value += m_source->phase(i);
        value += m_source->magnitudeRaw(i);
        coherence += m_source->coherence(i);
    };

    auto beforeSpline = [] (const auto * value, auto, const auto & count) {
        Complex c = value->m_phase / count;
        c /= c.abs();
        c *= value->m_magnitude / count;
        return c;
    };

    auto collected = [ &, this] (const float & f1, const float & f2, const Complex ac[4], const float c[4]) {

        if (m_openGL33CoreFunctions) {
            if (i + 12 > maxBufferSize) {
                return;
            }
            m_vertices[i + 0] = ac[0].real;
            m_vertices[i + 1] = ac[1].real;
            m_vertices[i + 2] = ac[2].real;
            m_vertices[i + 3] = ac[3].real;

            m_vertices[i + 4] = ac[0].imag;
            m_vertices[i + 5] = ac[1].imag;
            m_vertices[i + 6] = ac[2].imag;
            m_vertices[i + 7] = ac[3].imag;
            std::memcpy(m_vertices.data() + i + 8,  c, 4 * 4);
            verticiesCount ++;
            i += 12;
        } else {
            auto points = std::min(MAX_LINE_SPLITF, std::abs(std::round(f2 - f1)));
            float dt = 1.f / points;

            for (float t = 0; t < 1.0;) {

                auto x1 = ac[0].real + ac[1].real * t + ac[2].real * t * t + ac[3].real * t * t * t;
                auto y1 = ac[0].imag + ac[1].imag * t + ac[2].imag * t * t + ac[3].imag * t * t * t;
                auto c1 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

                t += dt;
                auto x2 = ac[0].real + ac[1].real * t + ac[2].real * t * t + ac[3].real * t * t * t;
                auto y2 = ac[0].imag + ac[1].imag * t + ac[2].imag * t * t + ac[3].imag * t * t * t;
                auto c2 = coherenceSpline(m_coherence, m_coherenceThreshold, c, t);

                addLineSegment(i, verticiesCount,
                               x1, y1,
                               x2, y2,
                               c1, c2);
            }
        }

        value.reset();
        coherence = 0.f;
    };

    iterateForSpline<NyquistPlot::SplineValue, Complex>(m_pointsPerOctave, &value, &coherence, accumulate, collected,
                                                        beforeSpline);

    m_program.setUniformValue(m_matrixUniform, m_matrix);
    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, m_weight * m_retinaScale);
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
            m_openGL33CoreFunctions->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(0));
            m_openGL33CoreFunctions->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(4 * sizeof(GLfloat)));
            m_openGL33CoreFunctions->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(8 * sizeof(GLfloat)));
        }
        m_openGL33CoreFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

        m_openGL33CoreFunctions->glEnableVertexAttribArray(0);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(1);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(2);
        m_openGL33CoreFunctions->glDrawArrays(GL_POINTS, 0, verticiesCount);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(2);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(1);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(0);

        m_refreshBuffers = false;
    } else {
        drawOpenGL2(verticiesCount);
    }
}

void NyquistSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<NyquistPlot *>(m_item ? m_item->parent() : nullptr)) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_coherence = plot->coherence();
        m_coherenceThreshold = plot->coherenceThreshold();
    }
}

void NyquistSeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMax, m_yMin, -1, 1);
}

const Shared::Source &NyquistSeriesRenderer::source() const
{
    return m_source;
}

} // namespace chart
