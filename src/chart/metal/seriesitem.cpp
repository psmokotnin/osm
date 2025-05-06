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
#include "seriesitem.h"
#include <QQuickWindow>

namespace Chart {

SeriesItem::SeriesItem(const Shared::Source &source, QQuickItem *parent,
                       NodeConstructor nodeConstructor) : QQuickItem(parent),
    m_source(source), m_highlighted(false), m_nodeConstructor(nodeConstructor)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setSize(parent->size());

    connect(source.get(), SIGNAL(colorChanged(QColor)),  SLOT(update()));
    connect(source.get(), SIGNAL(readyRead()),     SLOT(update()));
    connect(source.get(), SIGNAL(activeChanged()), SLOT(update()));
}

const Shared::Source &SeriesItem::source() const
{
    return m_source;
}

void SeriesItem::setZIndex(int index)
{
    setZ(index);
}

bool SeriesItem::highlighted() const
{
    return m_highlighted;
}

void SeriesItem::setHighlighted(const bool &highlighted)
{
    if (m_highlighted != highlighted) {
        m_highlighted = highlighted;
        update();
    }
}

QSGNode *SeriesItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    SeriesNode *node = static_cast<SeriesNode *>(oldNode);
    if (!node) {
        if (width() <= 0 || height() <= 0) {
            return nullptr;
        }
        node = m_nodeConstructor(this);
    }
    emit updated();
    return node;
}

void SeriesItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        update();
    }
}

} // namespace chart
