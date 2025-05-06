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

#include <QQmlEngine>
#include <QSGSimpleRectNode>
#include "chart/plot.h"
#include "source/group.h"

using namespace Chart;

CursorHelper *Plot::s_cursorHelper = new CursorHelper();

Plot::Plot(Settings *settings, QQuickItem *parent) :
    QQuickItem(parent), m_seriesesItem(this, this), m_settings(settings), m_palette(this), m_rendererError(),
    m_selectAppended(true)

{
    setFlag(QQuickItem::ItemHasContents, true);
    connect(parent, &QQuickItem::widthChanged, this, &Plot::parentWidthChanged);
    connect(parent, &QQuickItem::heightChanged, this, &Plot::parentHeightChanged);
    connect(&m_palette, SIGNAL(changed()), this, SLOT(update()));

    setWidth(parent->width());
    setHeight(parent->height());
}

void Plot::connectSources(SourceList *sourceList)
{
    m_seriesesItem.connectSources(sourceList);
}
void Plot::clear()
{
    m_seriesesItem.clear();
}
void Plot::disconnectFromParent()
{
    m_seriesesItem.disconnectFromParent();
    disconnect(parentItem(), &QQuickItem::widthChanged, this, &Plot::parentWidthChanged);
    disconnect(parentItem(), &QQuickItem::heightChanged, this, &Plot::parentHeightChanged);
}
void Plot::parentWidthChanged()
{
    if (!parentItem()) {
        return;
    }
    setWidth(parentItem()->width());
}

void Plot::parentHeightChanged()
{
    if (!parentItem()) {
        return;
    }
    setHeight(parentItem()->height());
}

CursorHelper *Plot::cursorHelper() const noexcept
{
    return s_cursorHelper;
}

bool Plot::selectAppended() const
{
    return m_selectAppended;
}

void Plot::setSelectAppended(bool selectAppended)
{
    m_selectAppended = selectAppended;
}

QList<QUuid> Plot::selected() const
{
    return m_selected;
}

void Plot::select(const QUuid &source)
{
    m_selected.push_back(source);
    emit selectedChanged();
}

void Plot::setSelected(const QList<QUuid> &selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        emit selectedChanged();
        update();
    }
}

bool Plot::isSelected(const QUuid &source) const
{
    if (source.isNull()) {
        return false;
    }
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

bool Plot::appendDataSource(const Shared::Source &source)
{
    if (!source) {
        return false;
    }
    m_seriesesItem.appendDataSource(source);

    if (m_selectAppended) {
        select(source.uuid());
    }
    return true;
}
void Plot::removeDataSource(const Shared::Source &source)
{
    m_seriesesItem.removeDataSource(source);
    m_selected.removeAll(source.uuid());
    emit selectedChanged();
}
void Plot::setSourceZIndex(const QUuid &source, int index)
{
    m_seriesesItem.setSourceZIndex(source, index);
}

void Plot::setHighlighted(const QUuid &source)
{
    m_seriesesItem.setHighlighted(source);
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
    m_seriesesItem.update();
}
