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

SourceModel::SourceModel(QObject *parent)
    : QAbstractListModel(parent), m_list(nullptr), m_filter(false),
      m_addNone(false), m_noneTitle("All")
{
}

int SourceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_list)
        return 0;
    return m_list->items().size();
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_list)
        return QVariant();

    chart::Source *source = m_list->items().at(index.row());
    QVariant r = {};
    switch (role) {
    case NameRole:
        r.setValue(source ? source->objectName() : m_noneTitle);
        break;

    case SourceRole:
        r.setValue(source);
        break;

    case TitleRole:
        r.setValue(source ? source->name() : m_noneTitle);
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
    return names;
}
void SourceModel::setList(SourceList *list)
{
    beginResetModel();

    if (m_filter || m_addNone) {
        list = list->clone(this, m_filter);
    }
    if (m_addNone) {
        list->appendNone();
    }

    if (m_list)
        m_list->disconnect(this);

    m_list = list;

    if (m_list) {
        connect(m_list, &SourceList::preItemAppended, this, [ = ]() {
            const int index = m_list->items().size();
            beginInsertRows(QModelIndex(), index, index);
        });
        connect(m_list, &SourceList::postItemAppended, this, [ = ](chart::Source *) {
            endInsertRows();
        });

        connect(m_list, &SourceList::preItemRemoved, this, [ = ](int index) {
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
    }

    endResetModel();
}
void SourceModel::setFilter(bool filter) noexcept
{
    m_filter = filter;
}

int SourceModel::indexOf(chart::Source *item) const noexcept
{
    return m_list->indexOf(item);
}

chart::Source *SourceModel::get(const int &index) const noexcept
{
    return m_list->get(index);
}
void SourceModel::setAddNone(bool addNone) noexcept
{
    m_addNone = addNone;
}

QString SourceModel::noneTitle() const
{
    return m_noneTitle;
}

void SourceModel::setNoneTitle(const QString &noneTitle)
{
    m_noneTitle = noneTitle;
}
