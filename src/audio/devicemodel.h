/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#ifndef AUDIO_DEVICEMODEL_H
#define AUDIO_DEVICEMODEL_H

#include <QSharedPointer>
#include <QAbstractListModel>

#include "deviceinfo.h"

namespace audio {
class Client;
class DeviceModel : public QAbstractListModel
{
public:
    enum DeviceScope {InputOnly, OutputOnly};

private:
    Q_OBJECT
    Q_ENUM(DeviceScope)
    Q_PROPERTY(DeviceScope scope READ scope WRITE setScope NOTIFY scopeChanged)

public:
    explicit DeviceModel(QObject *parent = nullptr);

    enum {
        IdRole      = Qt::UserRole + 1,
        NameRole    = Qt::UserRole + 2
    };

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    DeviceScope scope() const;
    void setScope(const DeviceScope &scope);

    Q_INVOKABLE audio::DeviceInfo::Id deviceId(const int &index);
    Q_INVOKABLE int indexOf(const audio::DeviceInfo::Id &deviceId);
    Q_INVOKABLE QStringList channelNames(const int &index);

private slots:
    void updateLists();

private:
    QSharedPointer<Client> m_client;
    DeviceScope m_scope;
    DeviceInfo::List m_list;

signals:
    void scopeChanged();

};

} // namespace audio

#endif // AUDIO_DEVICEMODEL_H
