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
#include "abstract/source.h"

class SourceList;

class SourceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(SourceList *list READ list WRITE setList NOTIFY listChanged)
    Q_PROPERTY(QList<QUuid> checked READ checked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(QUuid filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool addNone READ addNone WRITE setAddNone NOTIFY addNoneChanged)
    Q_PROPERTY(bool addAll READ addAll WRITE setAddAll NOTIFY addAllChanged)
    Q_PROPERTY(bool unrollGroups READ unrollGroups WRITE setUnrollGroups NOTIFY unrollGroupsChanged)
    Q_PROPERTY(QString noneTitle READ noneTitle WRITE setNoneTitle)
    Q_PROPERTY(QString allTitle READ allTitle WRITE setAllTitle)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit SourceModel(QObject *parent = nullptr);

    enum DataRole {
        SourceRole  = Qt::UserRole,
        NameRole    = Qt::UserRole + 1,
        TitleRole   = Qt::UserRole + 2,
        CheckedRole = Qt::UserRole + 3,
        ColorRole   = Qt::UserRole + 4,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    SourceList *list() const;
    void setList(SourceList *list);
    int count() const noexcept;

    Q_INVOKABLE int indexOf(const QUuid &item) const noexcept;
    Q_INVOKABLE QUuid get(const int &index) const noexcept;
    Q_INVOKABLE Shared::Source getShared(const int &index) const noexcept;

    Q_INVOKABLE void check(const int &index, const bool &checked) noexcept;
    Q_INVOKABLE int checkedCount() const;
    Q_INVOKABLE QUuid firstChecked() const noexcept;

    bool addNone() const noexcept;
    void setAddNone(bool addNone) noexcept;

    QString noneTitle() const;
    void setNoneTitle(const QString &noneTitle);

    bool addAll() const;
    void setAddAll(bool addAll);

    QString allTitle() const;
    void setAllTitle(const QString &allTitle);

    QList<QUuid> checked() const;
    void setChecked(const QList<QUuid> &selected);

    QUuid filter() const;
    void setFilter(const QUuid filter);

    bool unrollGroups() const;
    void setUnrollGroups(bool newUnrollGroups);

public slots:
    void itemChanged(const Shared::Source &, const QVector<int> &roles);

signals:
    void changed();
    void checkedChanged();
    void filterChanged();
    void listChanged();
    void addNoneChanged();
    void addAllChanged();

    void unrollGroupsChanged();
    void countChanged();

private:
    SourceList *m_list;
    QUuid m_filter;
    bool m_addNone, m_addAll, m_unrollGroups;
    int m_noneIndex, m_allIndex;
    QString m_noneTitle, m_allTitle;
};

#endif // SOURCEMODEL_H
