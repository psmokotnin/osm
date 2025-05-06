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
#include "seriesfbo.h"
#include "seriesrenderer.h"

using namespace Chart;

SeriesFBO::SeriesFBO(Shared::Source source, RendererCreator rc, QQuickItem *parent):
    QQuickFramebufferObject(parent),
    m_rendererCreator(std::move(rc)),
    m_source(source), m_highlighted(false)
{
    setFlag(QQuickItem::ItemHasContents);
    connect(source.get(), SIGNAL(colorChanged(QColor)), SLOT(update()));
    connect(source.get(), SIGNAL(readyRead()),          SLOT(update()));
    connect(source.get(), SIGNAL(activeChanged()),      SLOT(update()));
}

const Shared::Source &SeriesFBO::source() const noexcept
{
    return m_source;
}

QQuickFramebufferObject::Renderer *SeriesFBO::createRenderer() const
{
    SeriesRenderer *renderer = static_cast<SeriesRenderer *>(m_rendererCreator());
    auto connection = connect(this, &SeriesFBO::preSourceDeleted, this, [renderer]() {
        renderer->synchronize(nullptr);
    }, Qt::DirectConnection);

    renderer->setOnDelete([ this, connection ]() {
        disconnect(connection);
    });

    return renderer;
}
void SeriesFBO::setZIndex(qreal index)
{
    setZ(index);
}
bool SeriesFBO::highlighted() const noexcept
{
    return m_highlighted;
}
void SeriesFBO::setHighlighted(bool highlighted)
{
    if (m_highlighted != highlighted) {
        m_highlighted = highlighted;
        update();
    }
}
