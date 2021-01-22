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
namespace Fftchart {
class Source;
}

class SourceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(SourceList *list READ list WRITE setList)
    Q_PROPERTY(bool filter READ filter WRITE setFilter)
    Q_PROPERTY(bool addNone READ addNone WRITE setAddNone)

public:
    explicit SourceModel(QObject *parent = nullptr);

    enum {
        SourceRole = Qt::UserRole,
        NameRole    = 1,
        TitleRole   = 2
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    SourceList *list() const
    {
        return m_list;
    }
    void setList(SourceList *list);

    bool filter() const noexcept
    {
        return m_filter;
    }
    void setFilter(bool filter) noexcept;

    Q_INVOKABLE int indexOf(Fftchart::Source *item) const noexcept;
    Q_INVOKABLE Fftchart::Source *get(const int &index) const noexcept;

    bool addNone() const noexcept
    {
        return m_addNone;
    }
    void setAddNone(bool addNone) noexcept;

private:
    SourceList *m_list;
    bool m_filter;
    bool m_addNone;

signals:
    void changed();
};

#endif // SOURCEMODEL_H
