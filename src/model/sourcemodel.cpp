/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#include "sourcemodel.h"
#include "sourcelist.h"
#include "source/group.h"

SourceModel::SourceModel(QObject *parent)
    : QAbstractListModel(parent), m_list(nullptr), m_filter(),
      m_addNone(false), m_addAll(false), m_unrollGroups(false),
      m_noneIndex(-1), m_allIndex(-1),
      m_noneTitle("None"), m_allTitle("All")
{
}

int SourceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_list)
        return 0;
    return m_list->size();
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_list)
        return QVariant();

    auto j = index.row();
    const Shared::Source &source = m_list->get(j);
    QVariant r = {};
    switch (role) {
    case NameRole:
        r.setValue(source ? source->objectName() : m_noneTitle);
        break;

    case SourceRole:
        if (source) {
            r.setValue(source);
        }
        break;

    case TitleRole:
        if (j == m_noneIndex) {
            r.setValue(m_noneTitle);
        } else if (j == m_allIndex) {
            r.setValue(m_allTitle);
        } else {
            r.setValue(source ? source->name() : "");
        }
        break;

    case CheckedRole:
        r.setValue(m_list->isChecked(source ? source->uuid() : ""));
        break;

    case ColorRole:
        r.setValue(source ? source->color() : QColor());
        break;
    }

    return r;
}

Qt::ItemFlags SourceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable;
}

QHash<int, QByteArray> SourceModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[SourceRole]   = "source";
    names[NameRole]     = "name";
    names[TitleRole]    = "title";
    names[CheckedRole]  = "checked";
    names[ColorRole]    = "color";
    return names;
}

SourceList *SourceModel::list() const
{
    return m_list;
}

void SourceModel::setList(SourceList *list)
{
    beginResetModel();

    if (m_addAll || m_addNone || !m_filter.isNull() || m_unrollGroups) {
        list = list->clone(this, filter(), m_unrollGroups);
    }

    if (m_addNone) {
        m_noneIndex = list->appendNone();
    }
    if (m_addAll) {
        m_allIndex = list->appendAll();
        if (m_addNone) {
            m_noneIndex ++;
        }
    }

    if (m_list)
        m_list->disconnect(this);

    if (m_list != list) {
        m_list = list;
        emit listChanged();
    }

    if (m_list) {
        connect(m_list, &SourceList::preItemAppended, this, [ = ]() {
            const int index = m_list->size();
            beginInsertRows(QModelIndex(), index, index);
        });
        connect(m_list, &SourceList::postItemAppended, this, [ = ](auto) {
            endInsertRows();
        });

        connect(m_list, &SourceList::preItemRemoved, this, [ = ](auto uuid) {
            int index = m_list->getIndexByUUid(uuid);
            beginRemoveRows(QModelIndex(), index, index);
        });
        connect(m_list, &SourceList::postItemRemoved, this, [ = ]() {
            endRemoveRows();
        });
        connect(m_list, &SourceList::preItemMoved, this, [ = ](int from, int to) {
            beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
        });
        connect(m_list, &SourceList::postItemMoved, this, [ = ]() {
            endMoveRows();
        });

        connect(m_list, &SourceList::postItemChanged, this, &SourceModel::itemChanged);
        connect(m_list, &SourceList::countChanged, this, &SourceModel::countChanged);
    }
    endResetModel();
}

int SourceModel::count() const noexcept
{
    auto cnt = m_list ? m_list->count() : 0;
    if (m_noneIndex > -1) {
        cnt--;
    }
    if (m_allIndex > -1) {
        cnt--;
    }
    return std::max(cnt, 0);
}

int SourceModel::indexOf(const QUuid &item) const noexcept
{
    return m_list->indexOf(item);
}

QUuid SourceModel::get(const int &index) const noexcept
{
    return m_list->getUUid(index);
}

Shared::Source SourceModel::getShared(const int &index) const noexcept
{
    return m_list->get(index);
}

void SourceModel::check(const int &index, const bool &checked) noexcept
{
    if (index == m_allIndex) {
        m_list->checkAll();
    } else if (index == m_noneIndex) {
        m_list->uncheckAll();
    } else if (checked) {
        m_list->check(get(index));
    } else {
        m_list->uncheck(get(index));
    }
    emit checkedChanged();
}

int SourceModel::checkedCount() const
{
    return m_list->checkedCount();
}

QUuid SourceModel::firstChecked() const noexcept
{
    return m_list->firstChecked();
}

bool SourceModel::addNone() const noexcept
{
    return m_addNone;
}

void SourceModel::setAddNone(bool addNone) noexcept
{
    m_addNone = addNone;
    emit addNoneChanged();
}

QString SourceModel::noneTitle() const
{
    return m_noneTitle;
}

void SourceModel::setNoneTitle(const QString &noneTitle)
{
    m_noneTitle = noneTitle;
}

bool SourceModel::addAll() const
{
    return m_addAll;
}

void SourceModel::setAddAll(bool addAll)
{
    m_addAll = addAll;
    emit addAllChanged();
}

QString SourceModel::allTitle() const
{
    return m_allTitle;
}

void SourceModel::setAllTitle(const QString &allTitle)
{
    m_allTitle = allTitle;
}

QList<QUuid> SourceModel::checked() const
{
    return m_list->checked();
}

void SourceModel::setChecked(const QList<QUuid> &selected)
{
    m_list->setChecked(selected);
}

QUuid SourceModel::filter() const
{
    return m_filter;
}

void SourceModel::setFilter(const QUuid filter)
{
    if (m_filter != filter) {
        m_filter = filter;
        emit filterChanged();
    }
}

void SourceModel::itemChanged(const Shared::Source &item, const QVector<int> &roles)
{
    auto index = indexOf(item->uuid());
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), roles );
}

bool SourceModel::unrollGroups() const
{
    return m_unrollGroups;
}

void SourceModel::setUnrollGroups(bool newUnrollGroups)
{
    if (m_unrollGroups == newUnrollGroups)
        return;
    m_unrollGroups = newUnrollGroups;
    emit unrollGroupsChanged();
}
