/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#include "plot.h"

using namespace Fftchart;

SeriesRenderer::SeriesRenderer() :
    m_retinaScale(1),
    m_colorUniform(0),
    m_width(0), m_height(0), m_weight(2)
{
}
QOpenGLFramebufferObject *SeriesRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    m_openGLFunctions = QOpenGLContext::currentContext()->functions();
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
        m_weight = seriesFBO->highlighted() ? 3 : 1.5;

        auto plot = static_cast<Fftchart::Plot *>(m_item->parent());
        if (plot) {
            m_renderActive = !plot->filter() || plot->filter() == m_source;
        } else {
            m_renderActive = false;
        }
    }
}
void SeriesRenderer::render()
{
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
        m_source->lock();
        renderSeries();
        m_source->unlock();
    }

    m_program.release();
}

void SeriesRenderer::setWeight(unsigned int weight)
{
    m_weight = weight;
}
