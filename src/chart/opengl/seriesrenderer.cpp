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
#include "seriesrenderer.h"

#include <QQuickWindow>
#include "seriesfbo.h"
#include "../plot.h"
#include "common/profiler.h"
#include "chart/seriesesitem.h"
using namespace Chart;

SeriesRenderer::SeriesRenderer() :
    m_program(), m_openGLFunctions(nullptr), m_openGL33CoreFunctions(nullptr),
    m_retinaScale(1),
    m_colorUniform(0), m_positionAttribute(0),
    m_width(0), m_height(0), m_weight(2),
    m_renderActive(false),
    m_refreshBuffers(true),
    m_vertexBufferId(0), m_vertexArrayId(0),
    m_vertices(),
    m_active()
{
}
SeriesRenderer::~SeriesRenderer()
{
    if (m_onDelete) {
        m_onDelete();
    }
}
QOpenGLFramebufferObject *SeriesRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    if (!m_openGLFunctions) {
        m_openGLFunctions = QOpenGLContext::currentContext()->functions();
        m_openGLFunctions->initializeOpenGLFunctions();

        m_openGL33CoreFunctions = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (m_openGL33CoreFunctions) {
            m_openGL33CoreFunctions->initializeOpenGLFunctions();
        }
        init();
    }
    return new QOpenGLFramebufferObject(size, format);
}
void SeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    std::lock_guard<std::mutex> guard(m_active);
    m_item = item;
    bool reset = false;
    if (item) {
        auto seriesFBO = dynamic_cast<SeriesFBO *>(item);
        if (seriesFBO && (m_source = seriesFBO->source()) && seriesFBO->parentItem()) {
            qreal retinaScale = m_item->window()->devicePixelRatio();
            m_width  = static_cast<GLsizei>(m_item->width() * retinaScale);
            m_height = static_cast<GLsizei>(m_item->height() * retinaScale);
            m_retinaScale = static_cast<GLfloat>(retinaScale);
            auto currentPlot = plot();
            if (currentPlot && m_source && seriesFBO->parentItem()->isVisible()) {
                m_renderActive = currentPlot->isSelected(m_source->uuid());
                m_weight = currentPlot->palette().lineWidth(seriesFBO->highlighted());
            } else {
                m_renderActive = false;
            }
        } else {
            reset = true;
        }
    } else {
        reset = true;
    }

    if (reset) {
        m_renderActive = false;
        m_source.reset();
    }
}

float SeriesRenderer::coherenceSpline(const bool &coherence, const float &threshold, const float data[],
                                      const float &t) const
{
    if (!coherence) {
        return 1.f;
    }
    auto alpha = std::abs(data[0] + data[1] * t + data[2] * t * t + data[3] * t * t * t);
    if (alpha < threshold) {
        alpha = FLT_MIN;
    } else {
        float k = 1.0 / (1.0 - threshold + FLT_MIN);
        float b = -k * threshold;
        alpha = sqrt(k * alpha + b);
    }
    if (alpha > 1.f) {
        alpha = 1.f;
    }
    return alpha;
}

void SeriesRenderer::addLinePoint(unsigned int &i, unsigned int &verticiesCount,
                                  const float &x, const float &y, const float &c)
{
    if (m_vertices.size() < i + 6) {
        return;
    }
    m_vertices[i + 0] = x;
    m_vertices[i + 1] = y;

    m_vertices[i + 2] = static_cast<GLfloat>(m_source->color().redF());
    m_vertices[i + 3] = static_cast<GLfloat>(m_source->color().greenF());
    m_vertices[i + 4] = static_cast<GLfloat>(m_source->color().blueF());
    m_vertices[i + 5] = static_cast<GLfloat>(c);
    verticiesCount ++;
    i += 6;
}

void SeriesRenderer::addLineSegment(unsigned int &i, unsigned int &verticiesCount,
                                    const float &fromX, const float &fromY,
                                    const float &toX, const float &toY,
                                    const float &fromC, const float &toC)
{
    addLinePoint(i, verticiesCount, fromX, fromY, fromC);
    addLinePoint(i, verticiesCount, toX, toY, toC);
}

void SeriesRenderer::drawOpenGL2(unsigned int verticiesCount, GLenum mode)
{
    m_openGLFunctions->glLineWidth(m_weight * m_retinaScale);

    m_openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_positionAttribute), 2,
                                             GL_FLOAT, GL_FALSE, LINE_VERTEX_SIZE * sizeof(GLfloat),
                                             static_cast<const void *>(m_vertices.data()));
    m_openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_colorUniform), 4,
                                             GL_FLOAT, GL_FALSE, LINE_VERTEX_SIZE * sizeof(GLfloat),
                                             static_cast<const void *>(&m_vertices[2]));

    m_openGLFunctions->glEnableVertexAttribArray(0);
    m_openGLFunctions->glEnableVertexAttribArray(1);
    m_openGLFunctions->glDrawArrays(mode, 0, verticiesCount);
    m_openGLFunctions->glDisableVertexAttribArray(1);
    m_openGLFunctions->glDisableVertexAttribArray(0);
}

Plot *SeriesRenderer::plot() const
{
    if (m_item) {
        return dynamic_cast<Chart::Plot *>(m_item->parent());
    }
    return nullptr;
}

void SeriesRenderer::setOnDelete(const std::function<void ()> &newOnDelete)
{
    m_onDelete = newOnDelete;
}

void SeriesRenderer::render()
{
#ifdef QT_DEBUG
    Profiler p("SeriesRender");
#endif
    std::lock_guard<std::mutex> guard(m_active);
    if (!m_item) {
        return;
    }

    auto plot = static_cast<Chart::Plot *>(m_item->parent());
    if (!m_program.isLinked()) {
        qDebug() << QString("shader not setted or linked");
        return;
    }

    m_openGLFunctions->glViewport(
        0,
        0,
        m_width,
        m_height
    );

    m_openGLFunctions->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    m_openGLFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    m_openGLFunctions->glEnable(GL_BLEND);
    m_openGLFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_program.bind();

    m_program.setUniformValue(
        m_colorUniform,
        static_cast<GLfloat>(m_source->color().redF()),
        static_cast<GLfloat>(m_source->color().greenF()),
        static_cast<GLfloat>(m_source->color().blueF()),
        static_cast<GLfloat>(m_source->color().alphaF())
    );
    if (m_renderActive) {
        //m_openGL33CoreFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        m_source->lock();
        renderSeries();
        m_source->unlock();
        //m_openGL33CoreFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    m_program.release();
    plot->window()->resetOpenGLState();
}
void SeriesRenderer::setWeight(unsigned int weight)
{
    m_weight = weight;
}
