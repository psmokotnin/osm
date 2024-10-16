/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin

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

#include <QSGSimpleRectNode>
#include "chart/seriesesitem.h"
#include "chart/plot.h"
#include "source/group.h"

namespace chart {

SeriesesItem::SeriesesItem(QQuickItem *parent, Plot *plot) : QQuickItem(parent), m_plot(plot)
{
    connect(parent, &QQuickItem::widthChanged, this, &SeriesesItem::parentWidthChanged);
    connect(parent, &QQuickItem::heightChanged, this, &SeriesesItem::parentHeightChanged);
    setWidth(parent->width());
    setHeight(parent->height());
}

QSGNode *SeriesesItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto *node = static_cast<QSGSimpleRectNode *>(oldNode);
    if (!node) {
        node = new QSGSimpleRectNode();
    }
    node->setColor(QColor("transparent"));
    node->setRect(boundingRect());
    return node;
}

void SeriesesItem::disconnectFromParent()
{
    parentItem()->disconnect(this);
}

void SeriesesItem::clear()
{
    for (auto &&series : m_serieses) {
        emit series->preSourceDeleted();
        series->deleteLater();
    }
    m_serieses.clear();
    //TODO: clear subgroups
}

bool SeriesesItem::appendDataSource(const Source::Shared &source)
{
    if (!source || !m_plot) {
        return false;
    }
    auto it = std::find_if(m_serieses.begin(), m_serieses.end(), [ &source ](auto e) {
        if (e) {
            return (e->source().uuid() == source.uuid());
        }
        return false;
    });
    if (it != m_serieses.end()) {
        return false;
    }

    QQuickItem *item = nullptr;
    if (auto group = std::dynamic_pointer_cast<Source::Group>(source)) {
        item = new QQuickItem(this);
    } else {
        auto *sourceItem = m_plot->createSeriesFromSource(source);
        if (!sourceItem) {
            return false;
        }
        m_serieses.append(sourceItem);
        item = static_cast<QQuickItem *>(sourceItem);
    }

    applyWidthForSeries(item);
    applyHeightForSeries(item);

    return true;
}

void SeriesesItem::removeDataSource(const Source::Shared &source)
{
    foreach (auto *series, m_serieses) {
        if (series->source() == source) {
            emit series->preSourceDeleted();
            series->deleteLater();

            m_serieses.removeOne(series);
            update();
            return;
        }
    }
}

void SeriesesItem::setSourceZIndex(const QUuid &source, int index)
{
    foreach (SeriesItem *series, m_serieses) {
        if (series->source() && series->source()->uuid() == source) {
            series->setZIndex(index + 10);
            return;
        }
    }
}

void SeriesesItem::setHighlighted(const QUuid &source)
{
    foreach (SeriesItem *series, m_serieses) {
        series->setHighlighted((series->source() && series->source()->uuid() == source));
    }
}

void SeriesesItem::parentWidthChanged()
{
    if (!parentItem()) {
        return;
    }
    setWidth(parentItem()->width());
    for (auto &&series : m_serieses) {
        applyWidthForSeries(series);
    }
}

void SeriesesItem::parentHeightChanged()
{
    if (!parentItem()) {
        return;
    }
    setHeight(parentItem()->height());
    for (auto &&series : m_serieses) {
        applyHeightForSeries(series);
    }
}

void SeriesesItem::applyWidthForSeries(QQuickItem *s)
{
    if (!parentItem() || !m_plot)
        return;

    float width = static_cast<float>(parentItem()->width()) - m_plot->m_padding.left - m_plot->m_padding.right;
    s->setX(static_cast<qreal>(m_plot->m_padding.left));
    s->setWidth(static_cast<qreal>(width));
}

void SeriesesItem::applyHeightForSeries(QQuickItem *s)
{
    if (!parentItem() || !m_plot)
        return;

    float height = static_cast<float>(parentItem()->height()) - m_plot->m_padding.top - m_plot->m_padding.bottom;
    s->setY(static_cast<qreal>(m_plot->m_padding.top));
    s->setHeight(static_cast<qreal>(height));
}

void SeriesesItem::update()
{
    QQuickItem::update();

    foreach (SeriesItem *series, m_serieses) {
        series->update();
    }
}

} // namespace chart
