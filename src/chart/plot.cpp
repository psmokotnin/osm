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

CursorHelper *Plot::s_cursorHelper = new CursorHelper();

Plot::Plot(Settings *settings, QQuickItem *parent) :
    QQuickItem(parent), m_settings(settings), m_palette(this), m_rendererError(), m_selectAppended(true)
{
    connect(parent, &QQuickItem::widthChanged, this, &Plot::parentWidthChanged);
    connect(parent, &QQuickItem::heightChanged, this, &Plot::parentHeightChanged);
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
    disconnect(parentItem(), &QQuickItem::widthChanged, this, &Plot::parentWidthChanged);
    disconnect(parentItem(), &QQuickItem::heightChanged, this, &Plot::parentHeightChanged);
}
void Plot::parentWidthChanged()
{
    if (!parentItem()) {
        return;
    }
    setWidth(parentItem()->width());
    for (auto &&series : m_serieses) {
        applyWidthForSeries(series);
    }
}
void Plot::parentHeightChanged()
{
    if (!parentItem()) {
        return;
    }
    setHeight(parentItem()->height());
    for (auto &&series : m_serieses) {
        applyHeightForSeries(series);
    }
}
void Plot::applyWidthForSeries(SeriesItem *s)
{
    if (!parentItem())
        return;

    float width = static_cast<float>(parentItem()->width()) - m_padding.left - m_padding.right;
    s->setX(static_cast<qreal>(m_padding.left));
    s->setWidth(static_cast<qreal>(width));
}
void Plot::applyHeightForSeries(SeriesItem *s)
{
    if (!parentItem())
        return;

    float height = static_cast<float>(parentItem()->height()) - m_padding.top - m_padding.bottom;
    s->setY(static_cast<qreal>(m_padding.top));
    s->setHeight(static_cast<qreal>(height));
}

CursorHelper *Plot::cursorHelper() const noexcept
{
    return s_cursorHelper;
}

void Plot::setSelectAppended(bool selectAppended)
{
    m_selectAppended = selectAppended;
}

QList<Source *> Plot::selected() const
{
    QList<Source *> list;
    for (auto &item : m_selected) {
        list.push_back(item);
    }
    return list;
}

void Plot::select(Source *source)
{
    m_selected.push_back(source);
    emit selectedChanged();
}

void Plot::setSelected(const QList<Source *> selected)
{
    QList<QPointer<chart::Source>> list = {};
    for (auto item : selected) {
        list.push_back(item);
    }
    m_selected = list;
    emit selectedChanged();
    update();
}

bool Plot::isSelected(Source *source) const
{
    return m_selected.contains(source);
}

QString Plot::rendererError() const
{
    return m_rendererError;
}

void Plot::setRendererError(QString error)
{
    if (m_rendererError != error) {
        m_rendererError = error;
        emit rendererErrorChanged();
    }
}

QSGNode *Plot::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto *node = static_cast<QSGSimpleRectNode *>(oldNode);
    if (!node) {
        node = new QSGSimpleRectNode();
    }
    node->setColor(m_palette.backgroundColor());
    node->setRect(boundingRect());
    emit updated();
    return node;
}
void Plot::appendDataSource(Source *source)
{
    auto *sourceItem = createSeriesFromSource(source);
    if (sourceItem) {
        m_serieses.append(sourceItem);
        applyWidthForSeries(sourceItem);
        applyHeightForSeries(sourceItem);

        if (m_selectAppended) {
            select(source);
        }
    }
}
void Plot::removeDataSource(Source *source)
{
    foreach (auto *series, m_serieses) {
        if (series->source() == source) {
            emit series->preSourceDeleted();
            series->deleteLater();
            m_serieses.removeOne(series);
            m_selected.removeAll(source);
            emit selectedChanged();
            update();
            return;
        }
    }
}
void Plot::setSourceZIndex(Source *source, int index)
{
    foreach (SeriesItem *series, m_serieses) {
        if (series->source() == source) {
            series->setZIndex(index + 10);
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

void Plot::setSettings(Settings *settings) noexcept
{
    m_settings = settings;
}

const Palette &Plot::palette() const noexcept
{
    return m_palette;
}
bool Plot::darkMode() const noexcept
{
    return m_palette.darkMode();
}
void Plot::setDarkMode(bool darkMode) noexcept
{
    return m_palette.setDarkMode(darkMode);
}
void Plot::update()
{
    QQuickItem::update();

    foreach (SeriesItem *series, m_serieses) {
        series->update();
    }
}
