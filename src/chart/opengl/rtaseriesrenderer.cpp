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
#include "rtaseriesrenderer.h"
#include "../rtaplot.h"
#include "common/notifier.h"
#include "math/equalloudnesscontour.h"

using namespace Chart;
RTASeriesRenderer::RTASeriesRenderer() : FrequencyBasedSeriesRenderer(),
    m_showPeaks(false), m_inited(false), m_pointsPerOctave(0),
    m_mode(Chart::RTAPlot::Mode::Line),
    m_scale(Chart::RTAPlot::Scale::DBfs),
    m_vertexShader(nullptr),
    m_geometryShader(nullptr),
    m_fragmentShader(nullptr)
{
}

RTASeriesRenderer::~RTASeriesRenderer()
{
    delete m_vertexShader;
    delete m_geometryShader;
    delete m_fragmentShader;
}
void RTASeriesRenderer::init()
{
    if (m_openGL33CoreFunctions) {
        m_vertexShader   = new QOpenGLShader(QOpenGLShader::Vertex);
        m_geometryShader = new QOpenGLShader(QOpenGLShader::Geometry);
        m_fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);

        m_vertexShader->compileSourceFile(":/logx.vert");
        m_geometryShader->compileSourceFile(":/line.geom");
        m_fragmentShader->compileSourceFile(":/color.frag");
    } else {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl2/logx.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl2/color.frag");
    }

    m_inited = true;
    initShaders();
}
void RTASeriesRenderer::initShaders()
{
    if (!m_inited) {
        return;
    }

    if (m_openGL33CoreFunctions) {
        m_program.removeAllShaders();
        m_program.addShader(m_vertexShader);
        m_program.addShader(m_fragmentShader);
        if (m_mode != Chart::RTAPlot::Mode::Bars) {
            m_program.addShader(m_geometryShader);
        }
    }

    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("RTASeriesRenderer", m_program.log());
    }

    if (m_openGL33CoreFunctions) {
        m_colorUniform  = m_program.uniformLocation("m_color");
        m_matrixUniform = m_program.uniformLocation("matrix");
        if (m_mode != Chart::RTAPlot::Mode::Bars) {
            m_widthUniform  = m_program.uniformLocation("width");
            m_screenUniform = m_program.uniformLocation("screen");
        }
    } else {
        m_positionAttribute = m_program.attributeLocation("position");
        m_colorUniform  = m_program.attributeLocation("color");
        m_matrixUniform = m_program.uniformLocation("matrix");
    }
}
void RTASeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<RTAPlot *>(m_item ? m_item->parent() : nullptr)) {
        if (m_pointsPerOctave != plot->pointsPerOctave()) {
            m_refreshBuffers = true;
            m_pointsPerOctave = plot->pointsPerOctave();
        }
        if (m_mode != plot->mode()) {
            m_refreshBuffers = true;
            m_mode = plot->mode();
            initShaders();
        }
        if (m_scale != plot->scale()) {
            m_scale = plot->scale();
        }
        m_showPeaks = plot->showPeaks();
    }
}
void RTASeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMax, m_yMin, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), LEVEL_NORMALIZATION);
}
void RTASeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->frequencyDomainSize())
        return;

    m_program.setUniformValue(m_matrixUniform, m_matrix);
    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, m_weight * m_retinaScale);

    switch (m_mode) {
    case Chart::RTAPlot::Mode::Line:
        if (m_pointsPerOctave > 0) {
            renderPPOLine();
        } else {
            renderLine();
        }
        break;

    case Chart::RTAPlot::Mode::Bars:
        renderBars();
        break;

    case Chart::RTAPlot::Mode::Lines:
        renderLines();
        break;

    default:
        return;
    }
    m_refreshBuffers = false;
}
void RTASeriesRenderer::renderLine()
{
    unsigned int maxBufferSize = m_source->frequencyDomainSize() * (m_openGL33CoreFunctions ? 2 : VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE);
    unsigned int count = m_source->frequencyDomainSize();
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float offset = 0;
    switch (m_scale) {
    case RTAPlot::Scale::DBfs:
        offset = 0;
        break;
    case RTAPlot::Scale::SPL:
    case RTAPlot::Scale::Phon:
        offset = absolute_scale_offset;
    }

    Math::EqualLoudnessContour elc;
    if (m_openGL33CoreFunctions) {
        for (unsigned int i = 0, j = 0; i < count; ++i, j += 2) {
            auto value = 20 * log10f(m_source->module(i)) + offset;
            if (m_scale == RTAPlot::Scale::Phon) {
                value = elc.phone(m_source->frequency(i), value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            }

            m_vertices[j] = m_source->frequency(i);
            m_vertices[j + 1] = value;
        }

        drawVertices(count, GL_LINE_STRIP);
    } else {
        unsigned int j = 0, i, verticiesCount = 0;
        for (i = 0; i < m_source->frequencyDomainSize() - 1; ++i) {

            auto value1 = 20 * log10f(m_source->module(i)) + offset;
            auto value2 = 20 * log10f(m_source->module(i)) + offset;
            if (m_scale == RTAPlot::Scale::Phon) {
                value1 = elc.phone(m_source->frequency(i    ), value1 + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
                value2 = elc.phone(m_source->frequency(i + 1), value2 + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            }
            addLineSegment(j, verticiesCount,
                           m_source->frequency(i), value1,
                           m_source->frequency(i + 1), value2,
                           1, 1
                          );
        }
        drawOpenGL2(verticiesCount);
    }
}
void RTASeriesRenderer::renderPPOLine()
{
    unsigned int maxBufferSize = 12 * m_pointsPerOctave * (m_openGL33CoreFunctions ? 2 : VERTEX_PER_SEGMENT *
                                                           LINE_VERTEX_SIZE);
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float value = 0;
    float offset = 0;
    switch (m_scale) {
    case RTAPlot::Scale::DBfs:
        offset = 0;
        break;
    case RTAPlot::Scale::SPL:
    case RTAPlot::Scale::Phon:
        offset = absolute_scale_offset;
    }

    auto accumalte = [ &, this] (const unsigned int &i) {
        if (i == 0) {
            return ;
        }
        value += m_source->module(i) * m_source->module(i);
    };

    unsigned int i = 0, verticiesCount = 0;
    Math::EqualLoudnessContour elc;
    auto collected = [ &, this] (const float & start, const float & end, const unsigned int &) {

        auto frequencyPoint = (start + end) / 2;
        value = 10 * log10f(value) + offset;
        if (m_scale == RTAPlot::Scale::Phon) {
            value = elc.phone(frequencyPoint, value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
        }

        if (m_openGL33CoreFunctions) {
            if (i + 2 > maxBufferSize) {
                return;
            }
            m_vertices[i] = frequencyPoint;
            m_vertices[i + 1] = value;
            i += 2;
            verticiesCount++;
        } else {
            addLinePoint(i, verticiesCount, frequencyPoint, value, 1);
        }

        value = 0;
    };
    iterate(m_pointsPerOctave, accumalte, collected);

    if (m_openGL33CoreFunctions) {
        drawVertices(verticiesCount, GL_LINE_STRIP);
    } else {
        drawOpenGL2(verticiesCount, GL_LINE_STRIP);
    }
}
void RTASeriesRenderer::renderBars()
{
    unsigned int maxBufferSize = (m_pointsPerOctave ? m_pointsPerOctave * 12 : m_source->frequencyDomainSize()) *
                                 12 *
                                 (m_openGL33CoreFunctions ? 2 : LINE_VERTEX_SIZE);

    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    unsigned int verticesCollected = 0;

    float value = 0, peak = 0;
    float y = 1 / (m_matrix(0, 0) * m_width);
    float offset = 0;
    switch (m_scale) {
    case RTAPlot::Scale::DBfs:
        break;
    case RTAPlot::Scale::SPL:
    case RTAPlot::Scale::Phon:
        offset += absolute_scale_offset;
    }

    auto accumalte = [ &, this] (const unsigned int &i) {
        if (i == 0) {
            return ;
        }

        value += m_source->module(i) * m_source->module(i);
        peak = std::max(peak, m_source->peakSquared(i));
    };

    Math::EqualLoudnessContour elc;
    unsigned int i = 0;
    auto collected = [ &, this] (const float & start, const float & end, const unsigned int &) {

        value = 10 * log10f(value) + offset;
        peak  = 10 * log10f(peak) + offset;

        auto frequencyPoint = (start + end) / 2;
        if (m_scale == RTAPlot::Scale::Phon) {
            value = elc.phone(frequencyPoint, value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            peak  = elc.phone(frequencyPoint, peak + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
        }

        float shiftedStart = std::pow(M_E, std::log(start) + y);
        float shiftedEnd   = std::pow(M_E, std::log(end  ) - y);
        if (shiftedStart > shiftedEnd) {
            std::swap(shiftedStart, shiftedEnd);
        }
        if (shiftedEnd - shiftedStart < 1) {
            shiftedStart = start;
            shiftedEnd = end;
        }
        if (m_openGL33CoreFunctions) {
            if (i + 24 > maxBufferSize) {
                return;
            }
            m_vertices[i + 0] = shiftedStart;
            m_vertices[i + 1] = value;
            m_vertices[i + 2] = shiftedStart;
            m_vertices[i + 3] = m_yMin;
            m_vertices[i + 4] = shiftedEnd;
            m_vertices[i + 5] = value;

            m_vertices[i + 6] = shiftedEnd;
            m_vertices[i + 7] = value;
            m_vertices[i + 8] = shiftedEnd;
            m_vertices[i + 9] = m_yMin;
            m_vertices[i + 10] = shiftedStart;
            m_vertices[i + 11] = m_yMin;

            if (m_showPeaks) {
                m_vertices[i + 12] = shiftedStart;
                m_vertices[i + 13] = peak + 1;
                m_vertices[i + 14] = shiftedStart;
                m_vertices[i + 15] = peak;
                m_vertices[i + 16] = shiftedEnd;
                m_vertices[i + 17] = peak + 1;

                m_vertices[i + 18] = shiftedEnd;
                m_vertices[i + 19] = peak + 1;
                m_vertices[i + 20] = shiftedEnd;
                m_vertices[i + 21] = peak;
                m_vertices[i + 22] = shiftedStart;
                m_vertices[i + 23] = peak;

                i += 24;
                verticesCollected += 12;
            } else {
                i += 12;
                verticesCollected += 6;
            }
        } else {
            addLinePoint(i, verticesCollected, shiftedStart, value);
            addLinePoint(i, verticesCollected, shiftedStart, m_yMin);
            addLinePoint(i, verticesCollected, shiftedEnd, value);
            addLinePoint(i, verticesCollected, shiftedEnd, value);
            addLinePoint(i, verticesCollected, shiftedEnd, m_yMin);
            addLinePoint(i, verticesCollected, shiftedStart, m_yMin);

            if (m_showPeaks) {
                addLinePoint(i, verticesCollected, shiftedStart, peak + 1);
                addLinePoint(i, verticesCollected, shiftedStart, peak);
                addLinePoint(i, verticesCollected, shiftedEnd, peak + 1);
                addLinePoint(i, verticesCollected, shiftedEnd, peak + 1);
                addLinePoint(i, verticesCollected, shiftedEnd, peak);
                addLinePoint(i, verticesCollected, shiftedStart, peak);
            }
        }
        value = 0;
        peak = 0;
    };

    iterate(m_pointsPerOctave, accumalte, collected);
    if (m_openGL33CoreFunctions) {
        drawVertices(verticesCollected, GL_TRIANGLES);
    } else {
        drawOpenGL2(verticesCollected, GL_TRIANGLES);
    }
}
void RTASeriesRenderer::drawVertices(const GLsizei &count, const GLenum &mode)
{
    if (m_refreshBuffers) {
        m_openGL33CoreFunctions->glGenBuffers(1, &m_vertexBufferId);
        m_openGL33CoreFunctions->glGenVertexArrays(1, &m_vertexArrayId);
    }

    m_openGL33CoreFunctions->glBindVertexArray(m_vertexArrayId);
    m_openGL33CoreFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

    if (m_refreshBuffers) {
        m_openGL33CoreFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_vertices.size(), m_vertices.data(),
                                              GL_DYNAMIC_DRAW);
        m_openGL33CoreFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
                                                       reinterpret_cast<const void *>(0));
    }
    m_openGL33CoreFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(GLfloat) * count, m_vertices.data());

    m_openGL33CoreFunctions->glEnableVertexAttribArray(0);
    m_openGL33CoreFunctions->glDrawArrays(mode, 0, count);
    m_openGL33CoreFunctions->glDisableVertexAttribArray(0);
}
void RTASeriesRenderer::renderLines()
{
    unsigned int maxBufferSize = m_source->frequencyDomainSize() *
                                 (m_openGL33CoreFunctions ? 4 : 2 * VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE);
    unsigned int count = m_source->frequencyDomainSize();
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float offset = 0;
    Math::EqualLoudnessContour elc;
    switch (m_scale) {
    case RTAPlot::Scale::DBfs:
        offset = 0;
        break;
    case RTAPlot::Scale::SPL:
    case RTAPlot::Scale::Phon:
        offset = absolute_scale_offset;
        break;
    }

    if (m_openGL33CoreFunctions) {
        for (unsigned int i = 0, j = 0; i < count; ++i, j += 4) {

            auto value = 20 * log10f(m_source->module(i)) + offset;
            if (m_scale == RTAPlot::Scale::Phon) {
                value = elc.phone(m_source->frequency(i), value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            }

            m_vertices[j + 0] = m_source->frequency(i);
            m_vertices[j + 1] = m_yMin;
            m_vertices[j + 2] = m_source->frequency(i);
            m_vertices[j + 3] = value;
        }
        drawVertices(count * 2, GL_LINES);

        if (m_showPeaks) {
            for (unsigned int i = 0, j = 0; i < count; ++i, j += 4) {
                auto peak = 10 * log10f(m_source->peakSquared(i)) + offset;
                if (m_scale == RTAPlot::Scale::Phon) {
                    peak = elc.phone(m_source->frequency(i), peak + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
                }

                m_vertices[j + 1] = peak;
                m_vertices[j + 3] = peak + 1;
            }
            drawVertices(count * 2, GL_LINES);
        }
    } else {
        unsigned int verticiesCount = 0, j = 0;
        float peak;
        for (unsigned int i = 0; i < m_source->frequencyDomainSize(); ++i) {

            auto value = 20 * log10f(m_source->module(i)) + offset;
            if (m_scale == RTAPlot::Scale::Phon) {
                value = elc.phone(m_source->frequency(i), value + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
            }

            addLineSegment(j, verticiesCount,
                           m_source->frequency(i), m_yMin,
                           m_source->frequency(i), value,
                           1, 1
                          );

            if (m_showPeaks) {
                peak = 10 * log10f(m_source->peakSquared(i)) + offset;
                if (m_scale == RTAPlot::Scale::Phon) {
                    peak = elc.phone(m_source->frequency(i), peak + LEVEL_NORMALIZATION) - LEVEL_NORMALIZATION;
                }
                addLineSegment(j, verticiesCount,
                               m_source->frequency(i), peak,
                               m_source->frequency(i), peak + 1,
                               1, 1
                              );
            }
        }
        drawOpenGL2(verticiesCount);
    }
}
