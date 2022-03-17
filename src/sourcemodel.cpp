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
    : QAbstractListModel(parent), m_list(nullptr), m_filter(nullptr),
      m_addNone(false), m_addAll(false),
      m_noneIndex(-1), m_allIndex(-1),
      m_noneTitle("None"), m_allTitle("All")
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

    auto j = index.row();
    chart::Source *source = m_list->items().at(j);
    QVariant r = {};
    switch (role) {
    case NameRole:
        r.setValue(source ? source->objectName() : m_noneTitle);
        break;

    case SourceRole:
        r.setValue(source);
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
        r.setValue(m_list->isChecked(source));
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

    if (m_addAll || m_addNone || m_filter) {
        list = list->clone(this, filter());
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

int SourceModel::indexOf(chart::Source *item) const noexcept
{
    return m_list->indexOf(item);
}

chart::Source *SourceModel::get(const int &index) const noexcept
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

chart::Source *SourceModel::firstChecked() const noexcept
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
}

QString SourceModel::allTitle() const
{
    return m_allTitle;
}

void SourceModel::setAllTitle(const QString &allTitle)
{
    m_allTitle = allTitle;
}

QList<chart::Source *> SourceModel::checked() const
{
    return m_list->checked();
}

void SourceModel::setChecked(QList<chart::Source *> selected)
{
    m_list->setChecked(selected);
}

chart::Source *SourceModel::filter() const
{
    return m_filter;
}

void SourceModel::setFilter(chart::Source *filter)
{
    if (m_filter != filter) {
        m_filter = filter;
        emit filterChanged();
    }
}
