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
#include "rtaplot.h"

using namespace Fftchart;
RTASeriesRenderer::RTASeriesRenderer() : FrequencyBasedSeriesRenderer(),
    m_pointsPerOctave(0),
    m_mode(0),
    m_vertexShader(QOpenGLShader::Vertex),
    m_geometryShader(QOpenGLShader::Geometry),
    m_fragmentShader(QOpenGLShader::Fragment)
{
    m_vertexShader.compileSourceFile(":/logx.vert");
    m_geometryShader.compileSourceFile(":/line.geom");
    m_fragmentShader.compileSourceFile(":/color.frag");

    initShaders();
}
void RTASeriesRenderer::initShaders()
{
    m_program.removeAllShaders();
    m_program.addShader(&m_vertexShader);
    m_program.addShader(&m_fragmentShader);
    if (m_mode != 1) {
        m_program.addShader(&m_geometryShader);
    }
    m_program.link();

    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
    if (m_mode != 1) {
        m_widthUniform  = m_program.uniformLocation("width");
        m_screenUniform = m_program.uniformLocation("screen");
    }
}
void RTASeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);
    if (auto *plot = dynamic_cast<RTAPlot *>(m_item->parent())) {
        if (m_pointsPerOctave != plot->pointsPerOctave()) {
            m_refreshBuffers = true;
            m_pointsPerOctave = plot->pointsPerOctave();
        }
        if (m_mode != plot->mode()) {
            m_refreshBuffers = true;
            m_mode = plot->mode();
            initShaders();
        }
    }
}
void RTASeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMax, m_yMin, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), 0);
}
void RTASeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->size())
        return;

    m_program.setUniformValue(m_matrixUniform, m_matrix);
    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, m_weight * m_retinaScale);

    switch (m_mode) {
    //line
    case 0:
        renderLine();
        break;

    //bars
    case 1:
        renderBars();
        break;

    //lines
    case 2:
        renderLines();
        break;

    default:
        return;
    }
    m_refreshBuffers = false;
}
void RTASeriesRenderer::renderLine()
{
    unsigned int count = m_source->size();
    if (m_vertices.size() != count * 2) {
        m_vertices.resize(count * 2);
        m_refreshBuffers = true;
    }

    for (unsigned int i = 0, j = 0; i < count; ++i, j += 2) {
        m_vertices[j] = m_source->frequency(i);
        m_vertices[j + 1] = 20 * log10f(m_source->module(i) * .1f);
    }

    drawVertices(GL_LINE_STRIP, count);
}
void RTASeriesRenderer::renderBars()
{
    unsigned int maxBufferSize = m_pointsPerOctave * 11 * 8;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    unsigned int verticesCollected = 0;

    float value = 0;
    auto accumalte = [m_source = m_source, &value] (const unsigned int &i) {
        value += powf(m_source->module(i) * .1f, 2);
    };

    unsigned int i = 0;
    auto collected = [ &, this]
    (const float & start, const float & end, const unsigned int &) {
        if (i > maxBufferSize) {
            qCritical("out of range");
            return;
        }

        value = 10 * log10f(value);
        m_vertices[i + 0] = start;
        m_vertices[i + 1] = value;
        m_vertices[i + 2] = start;
        m_vertices[i + 3] = m_yMin;
        m_vertices[i + 4] = end;
        m_vertices[i + 5] = value;
        m_vertices[i + 6] = end;
        m_vertices[i + 7] = m_yMin;
        i += 8;
        verticesCollected += 4;
        value = 0;
    };

    iterate(m_pointsPerOctave, accumalte, collected);
    drawVertices(GL_TRIANGLE_STRIP, verticesCollected);
}
void RTASeriesRenderer::drawVertices(const GLenum &mode, const GLsizei &count)
{
    if (m_refreshBuffers) {
        m_openGLFunctions->glGenBuffers(1, &m_vertexBufferId);
        m_openGLFunctions->glGenVertexArrays(1, &m_vertexArrayId);
    }

    m_openGLFunctions->glBindVertexArray(m_vertexArrayId);
    m_openGLFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

    if (m_refreshBuffers) {
        m_openGLFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_vertices.size(), m_vertices.data(),
                                        GL_DYNAMIC_DRAW);
        m_openGLFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(0));
    }
    m_openGLFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(GLfloat) * count, m_vertices.data());

    m_openGLFunctions->glEnableVertexAttribArray(0);
    m_openGLFunctions->glDrawArrays(mode, 0, count);
    m_openGLFunctions->glDisableVertexAttribArray(0);
}
void RTASeriesRenderer::renderLines()
{
    unsigned int count = m_source->size();
    if (m_vertices.size() != count * 4) {
        m_vertices.resize(count * 4);
        m_refreshBuffers = true;
    }

    for (unsigned int i = 0, j = 0; i < count; ++i, j += 4) {
        m_vertices[j + 0] = m_source->frequency(i);
        m_vertices[j + 1] = m_yMin;
        m_vertices[j + 2] = m_source->frequency(i);
        m_vertices[j + 3] = 20 * log10f(m_source->module(i) * .1f);
    }

    drawVertices(GL_LINES, count * 2);
}
