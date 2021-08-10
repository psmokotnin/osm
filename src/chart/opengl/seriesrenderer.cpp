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
#include "profiler.h"
using namespace chart;

SeriesRenderer::SeriesRenderer() :
    m_retinaScale(1),
    m_colorUniform(0),
    m_width(0), m_height(0), m_weight(2),
    m_refreshBuffers(true),
    m_vertices()
{
}
QOpenGLFramebufferObject *SeriesRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    m_openGLFunctions = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    if (!m_openGLFunctions) {
        qDebug() << "QOpenGLFunctions_3_3_Core is not available";
    } else {
        m_openGLFunctions->initializeOpenGLFunctions();
    }
    return new QOpenGLFramebufferObject(size, format);
}
void SeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    m_item = item;
    auto seriesFBO = dynamic_cast<SeriesFBO *>(item);
    if ((m_source = seriesFBO->source())) {
        qreal retinaScale = m_item->window()->devicePixelRatio();
        m_width  = static_cast<GLsizei>(m_item->width() * retinaScale);
        m_height = static_cast<GLsizei>(m_item->height() * retinaScale);
        m_retinaScale = static_cast<GLfloat>(retinaScale);
        auto plot = static_cast<chart::Plot *>(m_item->parent());
        if (plot) {
            m_renderActive = !plot->filter() || plot->filter() == m_source;
            m_weight = plot->palette().lineWidth(seriesFBO->highlighted());
        } else {
            m_renderActive = false;
        }
    }
}
void SeriesRenderer::render()
{
#ifdef QT_DEBUG
    Profiler p("SeriesRender");
#endif
    if (!m_openGLFunctions) {
        auto plot = static_cast<chart::Plot *>(m_item->parent());
        plot->setRendererError("OpenGL 3.3 required");
        return;
    }
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
        //m_openGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        m_source->lock();
        renderSeries();
        m_source->unlock();
        //m_openGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    m_program.release();

    auto plot = static_cast<chart::Plot *>(m_item->parent());
    plot->window()->resetOpenGLState();
}
void SeriesRenderer::setWeight(unsigned int weight)
{
    m_weight = weight;
}
