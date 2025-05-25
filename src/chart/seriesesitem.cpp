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
#include "remote/items/groupitem.h"

namespace Chart {

SeriesesItem::SeriesesItem(QQuickItem *parent, Plot *plot) : QQuickItem(parent), m_plot(plot)
{
    setFlag(QQuickItem::ItemHasContents, true);
    connect(parent, &QQuickItem::widthChanged, this, &SeriesesItem::parentWidthChanged);
    connect(parent, &QQuickItem::heightChanged, this, &SeriesesItem::parentHeightChanged);
    setWidth(parent->width());
    setHeight(parent->height());
}

SeriesesItem::~SeriesesItem()
{
    clear();
}

void SeriesesItem::connectSources(SourceList *sourceList)
{
    if (!sourceList) {
        return;
    }
    m_sources = sourceList;
    if (m_sources && m_plot) {
        for (int i = 0; i < m_sources->count(); ++i) {
            auto source = m_sources->get(i);
            appendDataSource(source);
        }
        auto selected = m_sources->selectedUuid();
        m_plot->setHighlighted(selected);

        connect(m_sources, &SourceList::postItemAppended, this, [ = ](const Shared::Source & source) {
            appendDataSource(source);
        });

        connect(m_sources, &SourceList::preItemRemoved, this, [ = ](auto uuid) {
            auto source = m_sources->getByUUid(uuid);
            removeDataSource(source);
        });

        connect(sourceList, &SourceList::postItemMoved, this, &SeriesesItem::updateZOrders);

        connect(m_sources, &SourceList::selectedChanged, this, [this]() {
            if (m_plot) {
                updateZOrders();
                auto selected = m_sources->selectedUuid();
                m_plot->setHighlighted(selected);
                setHighlighted(selected);
                setSourceZIndex(selected, m_sources->count() + 1);
            }
        });
    }
}

template<typename GroupType>
SeriesesItem *SeriesesItem::constructFromGroup(const std::shared_ptr<GroupType> &group)
{
    if (!group) {
        return nullptr;
    }

    auto groupItem = new SeriesesItem(this, m_plot);
    groupItem->connectSources(group->sourceList());
    groupItem->setGroupUuid(group->uuid());

    connect(group.get(), &Abstract::Source::beforeDestroy, groupItem, [ = ](auto) {
        groupItem->clear();
        groupItem->deleteLater();
    });

    connect(group.get(), &::Abstract::Source::activeChanged, groupItem, [ = ]() {
        groupItem->setVisible(group->active());
        groupItem->update();
    });

    return groupItem;
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

    for (auto &child : childItems()) {
        if (auto child_serieses = dynamic_cast<SeriesesItem *>(child)) {
            child_serieses->clear();
            child_serieses->deleteLater();
        }
    }
}

bool SeriesesItem::appendDataSource(const Shared::Source &source)
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
        item = constructFromGroup(group);
    } else if (auto group = std::dynamic_pointer_cast<remote::GroupItem>(source)) {
        item = constructFromGroup(group);
    } else {
        auto *sourceItem = m_plot->createSeriesFromSource(source);
        if (!sourceItem) {
            return false;
        }
        sourceItem->setParentItem(this);
        m_serieses.append(sourceItem);
        item = static_cast<QQuickItem *>(sourceItem);
    }

    applyWidthForSeries(item);
    applyHeightForSeries(item);

    if (m_plot && m_plot->selectAppended()) {
        m_plot->select(source.uuid());
    }

    return true;
}

void SeriesesItem::removeDataSource(const Shared::Source &source)
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

    for (auto &child : childItems()) {
        if (auto child_serieses = dynamic_cast<SeriesesItem *>(child)) {
            if (source && child_serieses->groupUuid() == source->uuid()) {
                child_serieses->clear();
                child_serieses->deleteLater();
            }
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

Plot *SeriesesItem::plot() const
{
    return m_plot;
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

    qreal     x = m_plot->m_padding.left;
    qreal width = m_plot->width() - m_plot->m_padding.left - m_plot->m_padding.right;
    if (dynamic_cast<SeriesesItem *>(parentItem())) {
        x      = 0;
    }
    s->setX(x);
    s->setWidth(width);
}

void SeriesesItem::applyHeightForSeries(QQuickItem *s)
{
    if (!parentItem() || !m_plot)
        return;

    qreal y      = m_plot->m_padding.top;
    qreal height = m_plot->height() - m_plot->m_padding.top - m_plot->m_padding.bottom;
    if (dynamic_cast<SeriesesItem *>(parentItem())) {
        y       = 0;
    }
    s->setY(y);
    s->setHeight(height);
}

QUuid SeriesesItem::groupUuid() const
{
    return m_groupUuid;
}

void SeriesesItem::setGroupUuid(const QUuid &newUuid)
{
    m_groupUuid = newUuid;
}

void SeriesesItem::update()
{
    QQuickItem::update();

    foreach (SeriesItem *series, m_serieses) {
        series->update();
    }
    for (auto &child : childItems()) {
        if (auto serieses = dynamic_cast<SeriesesItem *>(child)) {
            serieses->update();
        }
    }
}

void SeriesesItem::updateZOrders()
{
    if (!m_plot || !m_sources) {
        return ;
    }
    auto total = m_sources->count();
    for (auto &&source : *m_sources) {
        if (source) {
            auto z = total - m_sources->indexOf(source);
            setSourceZIndex(source->uuid(), z);
        }
    }
    update();
}

} // namespace chart
