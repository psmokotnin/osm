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
#include "stepseriesrenderer.h"
#include "../stepplot.h"
#include "common/notifier.h"
using namespace Chart;

StepSeriesRenderer::StepSeriesRenderer() : XYSeriesRenderer(false, false), m_window(WindowFunction::Hann)
{
}

void StepSeriesRenderer::init()
{
    if (m_openGL33CoreFunctions) {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/pos.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/line.geom");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/color.frag");
    } else {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl2/point.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl2/color.frag");
    }
    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("StepSeriesRenderer", m_program.log());
    }
    if (m_openGL33CoreFunctions) {
        m_colorUniform = m_program.uniformLocation("m_color");
        m_matrixUniform = m_program.uniformLocation("matrix");
        m_widthUniform  = m_program.uniformLocation("width");
        m_screenUniform = m_program.uniformLocation("screen");
    } else {
        m_positionAttribute = m_program.attributeLocation("position");
        m_colorUniform  = m_program.attributeLocation("color");
        m_matrixUniform = m_program.uniformLocation("matrix");
    }
}

void StepSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->timeDomainSize())
        return;

    unsigned int maxBufferSize = m_source->timeDomainSize() * (m_openGL33CoreFunctions ? 4 : VERTEX_PER_SEGMENT *
                                                            LINE_VERTEX_SIZE), verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float res = 0.f;
    float offsetValue = 0;
    float dcOffset = 0;
    for (unsigned int i = 1; i < m_source->timeDomainSize() / 4; ++i) {
        dcOffset += m_source->impulseValue(i);
    }
    dcOffset /= m_source->timeDomainSize() / 4.0;
    for (unsigned int i = 1, j = 0; i < m_source->timeDomainSize() - 1; ++i) {
        res += m_source->impulseValue(i) - dcOffset;

        if (m_openGL33CoreFunctions) {
            m_vertices[j] = m_source->impulseTime(i);
            m_vertices[j + 1] = res;
            verticiesCount += 1;
            j += 2;
        } else {
            addLineSegment(j, verticiesCount,
                           m_source->impulseTime(i), res,
                           m_source->impulseTime(i + 1), res + m_source->impulseValue(i + 1) - dcOffset,
                           1, 1);
        }

        if (m_source->impulseTime(i) < m_zero) {
            offsetValue = res;
        }
    }

    updateMatrix();
    m_matrix.translate(QVector3D(0, -offsetValue, 0));
    m_program.setUniformValue(m_matrixUniform, m_matrix);
    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_program.setUniformValue(m_widthUniform, m_weight * m_retinaScale);

    if (m_openGL33CoreFunctions) {
        if (m_refreshBuffers) {
            m_openGL33CoreFunctions->glGenBuffers(1, &m_vertexBufferId);
            m_openGL33CoreFunctions->glGenVertexArrays(1, &m_vertexArrayId);
        }
        m_openGL33CoreFunctions->glBindVertexArray(m_vertexArrayId);
        m_openGL33CoreFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
        if (m_refreshBuffers) {
            m_openGL33CoreFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
            m_openGL33CoreFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(0));
        }
        m_openGL33CoreFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

        m_openGL33CoreFunctions->glEnableVertexAttribArray(0);
        m_openGL33CoreFunctions->glDrawArrays(GL_LINE_STRIP, 0, verticiesCount);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(0);

        m_refreshBuffers = false;
    } else {
        drawOpenGL2(verticiesCount);
    }
}

void StepSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);
    if (auto *plot = qobject_cast<StepPlot *>(m_item ? m_item->parent() : nullptr)) {
        m_zero = plot->zero();
    }
}

void StepSeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMax, m_yMin, -1, 1);
}
