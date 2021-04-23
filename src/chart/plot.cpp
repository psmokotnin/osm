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

#include "plot.h"
#include <QSGSimpleRectNode>
#include <QQmlEngine>

using namespace chart;

Plot::Plot(Settings *settings, QQuickItem *parent) :
    QQuickItem(parent), m_settings(settings), m_palette(this), m_filter(nullptr), m_openGLError(false)
{
    connect(parent, SIGNAL(widthChanged()), this, SLOT(parentWidthChanged()));
    connect(parent, SIGNAL(heightChanged()), this, SLOT(parentHeightChanged()));
    connect(&m_palette, SIGNAL(changed()), this, SLOT(update()));
    setWidth(parent->width());
    setHeight(parent->height());
}
void Plot::clear()
{
    for (auto &&series : m_serieses) {
        series->deleteLater();
    }
    m_serieses.clear();
}
void Plot::disconnectFromParent()
{
    disconnect(parentItem(), SIGNAL(widthChanged()), this, SLOT(parentWidthChanged()));
    disconnect(parentItem(), SIGNAL(heightChanged()), this, SLOT(parentHeightChanged()));
}
void Plot::parentWidthChanged()
{
    setWidth(parentItem()->width());
    foreach (SeriesItem *s, findChildren<SeriesItem *>()) {
        applyWidthForSeries(s);
    }
}
void Plot::parentHeightChanged()
{
    setHeight(parentItem()->height());
    foreach (SeriesItem *s, findChildren<SeriesItem *>()) {
        applyHeightForSeries(s);
    }
}
void Plot::applyWidthForSeries(SeriesItem *s)
{
    if (!parentItem())
        return;

    float width = static_cast<float>(parentItem()->width()) - padding.left - padding.right;
    s->setX(static_cast<qreal>(padding.left));
    s->setWidth(static_cast<qreal>(width));
}
void Plot::applyHeightForSeries(SeriesItem *s)
{
    if (!parentItem())
        return;

    float height = static_cast<float>(parentItem()->height()) - padding.top - padding.bottom;
    s->setY(static_cast<qreal>(padding.top));
    s->setHeight(static_cast<qreal>(height));
}

bool Plot::openGLError() const
{
    return m_openGLError;
}

void Plot::setOpenGLError(bool openGLError)
{
    if (m_openGLError != openGLError) {
        m_openGLError = openGLError;
        emit openGLErrorChanged();
    }
}
void Plot::setFilter(chart::Source *filter) noexcept
{
    if (m_filter != filter) {
        m_filter = filter;
        update();
        emit filterChanged(m_filter);
    }
}
QSGNode *Plot::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto *node = dynamic_cast<QSGSimpleRectNode *>(oldNode);
    if (!node) {
        node = new QSGSimpleRectNode();
    }
    node->setColor(m_palette.backgroundColor());
    node->setRect(boundingRect());
    return node;
}
void Plot::appendDataSource(Source *source)
{
    auto *sourceItem = createSeriesFromSource(source);
    if (sourceItem) {
        m_serieses.append(sourceItem);
        applyWidthForSeries(sourceItem);
        applyHeightForSeries(sourceItem);
    }
}
void Plot::removeDataSource(Source *source)
{
    foreach (auto *series, m_serieses) {
        if (series->source() == source) {
            series->deleteLater();
            m_serieses.removeOne(series);
            update();
            return;
        }
    }
}
void Plot::setSourceZIndex(Source *source, int index)
{
    foreach (SeriesItem *series, m_serieses) {
        if (series->source() == source) {
            series->setZIndex(index);
            return;
        }
    }
}

void Plot::setHighlighted(Source *source)
{
    foreach (SeriesItem *series, m_serieses) {
        series->setHighlighted((series->source() == source));
    }
}
void Plot::update()
{
    QQuickItem::update();
    emit updated();

    foreach (SeriesItem *series, m_serieses) {
        series->update();
    }
}
