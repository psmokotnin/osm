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
#include "seriesfbo.h"
#include <QSGSimpleRectNode>
#include <QQmlEngine>

using namespace Fftchart;

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
    for (auto &&seriesfbo : series) {
        seriesfbo->deleteLater();
    }
    series.clear();
}
void Plot::disconnectFromParent()
{
    disconnect(parentItem(), SIGNAL(widthChanged()), this, SLOT(parentWidthChanged()));
    disconnect(parentItem(), SIGNAL(heightChanged()), this, SLOT(parentHeightChanged()));
}
void Plot::parentWidthChanged()
{
    setWidth(parentItem()->width());
    foreach (SeriesFBO *s, findChildren<SeriesFBO *>()) {
        applyWidthForSeries(s);
    }
}
void Plot::parentHeightChanged()
{
    setHeight(parentItem()->height());
    foreach (SeriesFBO *s, findChildren<SeriesFBO *>()) {
        applyHeightForSeries(s);
    }
}
void Plot::applyWidthForSeries(SeriesFBO *s)
{
    if (!parentItem())
        return;

    float width = static_cast<float>(parentItem()->width()) - padding.left - padding.right;
    s->setX(static_cast<qreal>(padding.left));
    s->setWidth(static_cast<qreal>(width));
}
void Plot::applyHeightForSeries(SeriesFBO *s)
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
void Plot::setFilter(Fftchart::Source *filter) noexcept
{
    if (m_filter != filter) {
        m_filter = filter;
        update();
        emit filterChanged(m_filter);
    }
}
QSGNode *Plot::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto *n = dynamic_cast<QSGSimpleRectNode *>(oldNode);
    if (!n) {
        n = new QSGSimpleRectNode();
    }
    n->setColor(m_palette.backgroundColor());
    n->setRect(boundingRect());
    return n;
}
void Plot::appendDataSource(Source *source)
{
    SeriesFBO *s = createSeriesFromSource(source);
    series.append(s);
    applyWidthForSeries(s);
    applyHeightForSeries(s);
}
void Plot::removeDataSource(Source *source)
{
    foreach (SeriesFBO *seriesfbo, series) {
        if (seriesfbo->source() == source) {
            seriesfbo->deleteLater();
            series.removeOne(seriesfbo);
            update();
            return;
        }
    }
}
void Plot::setSourceZIndex(Source *source, int index)
{
    foreach (SeriesFBO *seriesfbo, series) {
        if (seriesfbo->source() == source) {
            seriesfbo->setZIndex(index);
            return;
        }
    }
}

void Plot::setHighlighted(Source *source)
{
    foreach (SeriesFBO *seriesfbo, series) {
        seriesfbo->setHighlighted((seriesfbo->source() == source));
    }
}
void Plot::update()
{
    QQuickItem::update();
    emit updated();

    foreach (SeriesFBO *seriesfbo, series) {
        seriesfbo->update();
    }
}
