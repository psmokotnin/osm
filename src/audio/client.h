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
#ifndef AUDIO_CLIENT_H
#define AUDIO_CLIENT_H

#include <QObject>
#include <QList>
#include <QIODevice>
#include <QSharedPointer>

#include "plugin.h"

namespace audio {

class Plugin;
class Client : public QObject
{
    Q_OBJECT

public:
    static QSharedPointer<Client> getInstance();
    static DeviceInfo::Id defaultInputDeviceId();
    static DeviceInfo::Id defaultOutputDeviceId();

    DeviceInfo::List getDeviceList() const;
    DeviceInfo::Id defaultDeviceId(const Plugin::Direction &mode) const;

    bool isDevicePresent(const DeviceInfo::Id &id) const;
    QString deviceName(const DeviceInfo::Id &id) const;
    DeviceInfo::Id deviceIdByName(const QString &name, const Plugin::Direction direction) const;

    Format deviceOutputFormat(const DeviceInfo::Id &id) const;
    Stream *openOutput(const DeviceInfo::Id &id, QIODevice *source, const Format &format);

    Format deviceInputFormat(const DeviceInfo::Id &id) const;
    Stream *openInput(const DeviceInfo::Id &id, QIODevice *source, const Format &format);

private:
    explicit Client();
    void initPlugins();
    void refreshDeviceList();
    QSharedPointer<Plugin> plugin(const QString &name) const;
    QSharedPointer<Plugin> pluginForDevice(const DeviceInfo::Id &id) const;

    static QSharedPointer<Client> m_instance;
    QList<QSharedPointer<Plugin>> m_plugins;
    DeviceInfo::List m_deviceList;

signals:
    void deviceListChanged();
};

} // namespace audio

#endif // AUDIO_CLIENT_H
