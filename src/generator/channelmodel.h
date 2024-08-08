/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QAbstractListModel>
#include <QtQml>
#include "generator.h"

class GeneratorChannelModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList list READ list WRITE setList NOTIFY listChanged)
    Q_PROPERTY(QStringList checked READ checked NOTIFY checkedChanged)
    Q_PROPERTY(QSet<int> checkedIndexes READ checkedIndexes NOTIFY checkedChanged)
    Q_PROPERTY(QString noneTitle MEMBER m_noneTitle)
    Q_PROPERTY(QString allTitle  MEMBER m_allTitle )
    Q_PROPERTY(Generator *generator  MEMBER m_generator )

    QML_ELEMENT

public:
    explicit GeneratorChannelModel(QObject *parent = nullptr);

    enum {
        NameRole    = Qt::UserRole + 0,
        TitleRole   = Qt::UserRole + 1,
        CheckedRole = Qt::UserRole + 2,
        CheckedAbleRole = Qt::UserRole + 3,
        //ColorRole   = Qt::UserRole + 3
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    QStringList list() const;
    void setList(const QStringList &data);

    Q_INVOKABLE void check(const int &index, const bool &checked) noexcept;
    Q_INVOKABLE int checkedCount() const;
    QStringList checked() const;
    QSet<int> checkedIndexes() const;

signals:
    void checkedChanged();
    void listChanged();

private:
    void checkAll(const bool &checked);

    QStringList m_list;
    std::vector<std::tuple<QString, bool, int>> m_data;
    QString m_noneTitle, m_allTitle;
    Generator *m_generator;

    static const unsigned int m_allIndex = 0, m_noneIndex = 1;

};

#endif // CHANNELMODEL_H
