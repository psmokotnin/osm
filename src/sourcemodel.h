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
namespace chart {
class Source;
}

class SourceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(SourceList *list READ list WRITE setList)
    Q_PROPERTY(QList<chart::Source *> checked READ checked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(chart::Source *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool addNone READ addNone WRITE setAddNone)
    Q_PROPERTY(bool addAll READ addAll WRITE setAddAll)
    Q_PROPERTY(QString noneTitle READ noneTitle WRITE setNoneTitle)
    Q_PROPERTY(QString allTitle READ allTitle WRITE setAllTitle)

public:
    explicit SourceModel(QObject *parent = nullptr);

    enum {
        SourceRole  = Qt::UserRole,
        NameRole    = Qt::UserRole + 1,
        TitleRole   = Qt::UserRole + 2,
        CheckedRole = Qt::UserRole + 3,
        ColorRole   = Qt::UserRole + 4
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    SourceList *list() const;
    void setList(SourceList *list);

    Q_INVOKABLE int indexOf(chart::Source *item) const noexcept;
    Q_INVOKABLE chart::Source *get(const int &index) const noexcept;

    Q_INVOKABLE void check(const int &index, const bool &checked) noexcept;
    Q_INVOKABLE int checkedCount() const;
    Q_INVOKABLE chart::Source *firstChecked() const noexcept;

    bool addNone() const noexcept;
    void setAddNone(bool addNone) noexcept;

    QString noneTitle() const;
    void setNoneTitle(const QString &noneTitle);

    bool addAll() const;
    void setAddAll(bool addAll);

    QString allTitle() const;
    void setAllTitle(const QString &allTitle);

    QList<chart::Source *> checked() const;
    void setChecked(QList<chart::Source *> selected);

    chart::Source *filter() const;
    void setFilter(chart::Source *filter);

signals:
    void changed();
    void checkedChanged();
    void filterChanged();

private:
    SourceList *m_list;
    chart::Source *m_filter;
    bool m_addNone, m_addAll;
    int m_noneIndex, m_allIndex;
    QString m_noneTitle, m_allTitle;

};

#endif // SOURCEMODEL_H
