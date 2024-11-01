/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#include "levelseriesrenderer.h"
#include "common/notifier.h"
#include "measurement.h"

namespace Chart {

LevelSeriesRenderer::LevelSeriesRenderer(): XYSeriesRenderer(false, false), m_timer(), m_history(0)
{
    m_timer.restart();
}

void LevelSeriesRenderer::init()
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
        emit Notifier::getInstance()->newMessage("LevelSeriesRenderer", m_program.log());
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


void LevelSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<LevelPlot *>(m_item ? m_item->parent() : nullptr)) {

        if (m_curve != plot->curve() || m_time != plot->time() || m_mode != plot->mode()) {

            m_curve = plot->curve();
            m_time = plot->time();
            m_mode = plot->mode();

            m_history.clear();
        }
        m_pause = plot->pause();
    }
}

void LevelSeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(m_xMin, m_xMax, m_yMax, m_yMin, -1, 1);
}

void LevelSeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    if (m_timer.elapsed() >= Measurement::TIMER_INTERVAL && !m_pause) {
        auto time = static_cast<float>(m_timer.restart()) / 1000.f;
        auto level = m_source->level(m_curve, m_time);
        if (m_mode == LevelPlot::SPL) {
            level += LevelPlot::SPL_OFFSET;
        }
        m_history.push_back( {time, level});
    }

    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.pop_front();
    }

    unsigned int maxBufferSize =
        MAX_HISTORY_SIZE * (m_openGL33CoreFunctions ? 4 : VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE),
        verticiesCount = 0;

    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float time = 0, level = 0;
    unsigned int j = 0, i = 0;
    timePoint lastPoint = {0, 0};

    for (auto point = m_history.crbegin(); point != m_history.crend(); ++point) {
        time -= point->time;
        level = point->value;
        ++i;

        if (m_openGL33CoreFunctions) {
            m_vertices[j] = time;
            m_vertices[j + 1] = level;
            verticiesCount += 1;
            j += 2;
        } else {
            if (lastPoint.time < 0) {
                addLineSegment(j, verticiesCount,
                               lastPoint.time, lastPoint.value,
                               time, level,
                               1, 1);
            }
            lastPoint.time = time;
            lastPoint.value = level;
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

} // namespace chart
