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
#include "impulseseriesrenderer.h"
#include "common/notifier.h"
using namespace Chart;

ImpulseSeriesRenderer::ImpulseSeriesRenderer() : XYSeriesRenderer(false, false),
    m_widthUniform(0), m_screenUniform(0), m_mode(ImpulsePlot::Mode::Linear)
{
}
void ImpulseSeriesRenderer::init()
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
        emit Notifier::getInstance()->newMessage("ImpulseSeriesRenderer", m_program.log());
    }

    if (m_openGL33CoreFunctions) {
        m_colorUniform  = m_program.uniformLocation("m_color");
        m_matrixUniform = m_program.uniformLocation("matrix");
        m_widthUniform  = m_program.uniformLocation("width");
        m_screenUniform = m_program.uniformLocation("screen");
    } else {
        m_positionAttribute = m_program.attributeLocation("position");
        m_colorUniform  = m_program.attributeLocation("color");
        m_matrixUniform = m_program.uniformLocation("matrix");
    }
}
void ImpulseSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->timeDomainSize())
        return;

    unsigned int maxBufferSize = m_source->timeDomainSize() * (m_openGL33CoreFunctions ? 4 : VERTEX_PER_SEGMENT *
                                                            LINE_VERTEX_SIZE), verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float max = 0;
    if (m_normalized) {
        for (unsigned int i = 0; i < m_source->timeDomainSize(); ++i) {
            max = std::max(max, std::abs(m_source->impulseValue(i)));
        }
    } else {
        max = 1;
    }

    float dc =  (m_source->impulseValue(0) + m_source->impulseValue(m_source->timeDomainSize() - 1)) / 2;
    dc /= max;
    float value = 0, lastValue = 0, lastTime = 0;
    for (unsigned int i = 0, j = 0; i < m_source->timeDomainSize(); ++i) {

        switch (m_mode) {
        case ImpulsePlot::Linear:
            value = m_source->impulseValue(i) / max - dc;
            break;
        case ImpulsePlot::Log:
            value = 10 * std::log10(std::pow(m_source->impulseValue(i) / max - dc, 2));
            break;
        }

        if (m_openGL33CoreFunctions) {
            m_vertices[j]     = m_source->impulseTime(i);
            m_vertices[j + 1] = value;
            verticiesCount += 1;
            j += 2;
        } else {
            if (i > 0) {
                addLineSegment(j, verticiesCount,
                               lastTime,                 lastValue,
                               m_source->impulseTime(i), value,
                               1, 1);
            }
            lastValue = value;
            lastTime = m_source->impulseTime(i);
        }
    }

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

void ImpulseSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<ImpulsePlot *>(m_item ? m_item->parent() : nullptr)) {
        m_mode = plot->mode();
        m_normalized = plot->normalized();
    }

}

void ImpulseSeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMax, m_yMin, -1, 1);
}
