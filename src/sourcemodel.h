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
#ifndef SOURCEMODEL_H
#define SOURCEMODEL_H

#include <QAbstractListModel>

class SourceList;

class SourceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(SourceList *list READ list WRITE setList)

public:
    explicit SourceModel(QObject *parent = nullptr);

    enum {
            SourceRole = Qt::UserRole,
            NameRole    = 1
        };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    SourceList *list() const {return mList;}
    void setList(SourceList *list);

private:
    SourceList *mList;
};

#endif // SOURCEMODEL_H
