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
    : QAbstractListModel(parent), mList(nullptr)
{
}

int SourceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !mList)
        return 0;

    return mList->items().size();
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !mList)
        return QVariant();

    Fftchart::Source* source = mList->items().at(index.row());
    QVariant r;
    switch (role) {
        case NameRole:
            r.setValue(source->objectName());
        break;

        case SourceRole:
            r.setValue(source);
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
    return names;
}
void SourceModel::setList(SourceList *list)
{
    beginResetModel();

    if (mList)
        mList->disconnect(this);

    mList = list;

    if (mList) {
        connect(mList, &SourceList::preItemAppended, this, [=]() {
            const int index = mList->items().size();
            beginInsertRows(QModelIndex(), index, index);
        });
        connect(mList, &SourceList::postItemAppended, this, [=](Fftchart::Source *) {
            endInsertRows();
        });

        connect(mList, &SourceList::preItemRemoved, this, [=](int index) {
            beginRemoveRows(QModelIndex(), index, index);
        });
        connect(mList, &SourceList::postItemRemoved, this, [=]() {
            endRemoveRows();
        });
    }

    endResetModel();
}
