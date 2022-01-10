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
#include "client.h"
#include "deviceinfo.h"
#include <QtCore>

#ifdef Q_OS_MACOS
#include "plugins/coreaudio.h"
#endif

#ifdef Q_OS_IOS
#include "plugins/audiosession.h"
#endif

#ifdef Q_OS_WINDOWS
#include "plugins/wasapi.h"
#endif

#ifdef Q_OS_LINUX
#include "plugins/alsa.h"
#endif

#ifdef USE_ASIO
#include "plugins/asioplugin.h"
#endif

namespace audio {

QSharedPointer<Client> Client::m_instance = nullptr;

QSharedPointer<Client> Client::getInstance()
{
    if (!m_instance) {
        m_instance = QSharedPointer<Client>(new Client());
    }
    return m_instance;
}

DeviceInfo::Id Client::defaultInputDeviceId()
{
    return getInstance()->defaultDeviceId(Plugin::Direction::Input);
}

DeviceInfo::Id Client::defaultOutputDeviceId()
{
    return getInstance()->defaultDeviceId(Plugin::Direction::Output);
}

Client::Client() : QObject(), m_plugins(), m_deviceList()
{
    initPlugins();
    qRegisterMetaType<audio::DeviceInfo::Id>("audio::DeviceInfo::Id");
}

void Client::initPlugins()
{
#ifdef Q_OS_MACOS
    m_plugins.push_back(QSharedPointer<Plugin>(new CoreaudioPlugin()));
#endif

#ifdef Q_OS_IOS
    m_plugins.push_back(QSharedPointer<Plugin>(new AudioSessionPlugin()));
#endif

#ifdef USE_ASIO
    m_plugins.push_back(QSharedPointer<Plugin>(new ASIOPlugin()));
#endif

#ifdef Q_OS_WINDOWS
    m_plugins.push_back(QSharedPointer<Plugin>(new WasapiPlugin()));
#endif

#ifdef Q_OS_LINUX
    m_plugins.push_back(QSharedPointer<Plugin>(new AlsaPlugin()));
#endif

    for (auto &&plugin : m_plugins) {
        connect(plugin.data(), &Plugin::deviceListChanged, this, [this]() {
            refreshDeviceList();
        });
    }
    refreshDeviceList();
}

void Client::refreshDeviceList()
{
    DeviceInfo::List list;
    for (auto &&plugin : m_plugins) {
        for (auto &device : plugin->getDeviceInfoList()) {
            list.push_back(device);
        }
    }
    m_deviceList = list;
    std::sort(m_deviceList.begin(), m_deviceList.end(), [](auto a, auto b) {
        return a.name() < b.name();
    });
    emit deviceListChanged();
}

QSharedPointer<Plugin> Client::plugin(const QString &name) const
{
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [name](const auto & e) {
        return e->name() == name;
    });

    if (it == m_plugins.end()) {
        qCritical() << "Plugin " << name << " not found";
        return nullptr;
    }
    return *it;
}

DeviceInfo::List Client::getDeviceList() const
{
    return m_deviceList;
}

DeviceInfo::Id Client::defaultDeviceId(const Plugin::Direction &mode) const
{
    for (auto &plugin : m_plugins) {
        auto device = plugin->defaultDeviceId(mode);
        if (!device.isEmpty()) {
            return device;
        }
    }
    qCritical() << "Can't get default device";
    return DeviceInfo::Id();
}

bool Client::isDevicePresent(const DeviceInfo::Id &id) const
{
    auto it = std::find_if(m_deviceList.begin(), m_deviceList.end(), [&id](const auto & e) {
        return e.id() == id;
    });
    return it != m_deviceList.end();
}

QString Client::deviceName(const DeviceInfo::Id &id) const
{
    auto it = std::find_if(m_deviceList.begin(), m_deviceList.end(), [&id](const auto & e) {
        return e.id() == id;
    });
    if (it != m_deviceList.end()) {
        return (*it).name();
    }
    return {};
}

DeviceInfo::Id Client::deviceIdByName(const QString &name, const Plugin::Direction direction) const
{
    auto it = std::find_if(m_deviceList.begin(), m_deviceList.end(), [&name, &direction](const auto & e) {
        if (
            (direction == Plugin::Input && e.inputChannels().count() > 0) ||
            (direction == Plugin::Output && e.outputChannels().count() > 0)) {
            return e.name() == name;
        }
        return false;
    });
    if (it != m_deviceList.end()) {
        return (*it).id();
    }
    return {};
}

Format Client::deviceOutputFormat(const DeviceInfo::Id &id) const
{
    auto targetPlugin = pluginForDevice(id);
    if (!targetPlugin) {
        return {};
    }
    return targetPlugin->deviceFormat(id, Plugin::Direction::Output);
}

QSharedPointer<Plugin> Client::pluginForDevice(const DeviceInfo::Id &id) const
{
    auto it = std::find_if(m_deviceList.begin(), m_deviceList.end(), [id](const auto & e) {
        return e.id() == id;
    });

    if (it == m_deviceList.end()) {
        qDebug() << "Audio device (" + id + ")not found";
        return nullptr;
    }

    return plugin(it->pluginName());
}

Stream *Client::openOutput(const DeviceInfo::Id &id, QIODevice *source, const Format &format)
{
    auto targetPlugin = pluginForDevice(id);
    if (!targetPlugin) {
        return nullptr;
    }
    return targetPlugin->open(id, Plugin::Direction::Output, format, source);
}

Format Client::deviceInputFormat(const DeviceInfo::Id &id) const
{
    auto targetPlugin = pluginForDevice(id);
    if (!targetPlugin) {
        return {};
    }
    return targetPlugin->deviceFormat(id, Plugin::Direction::Input);
}

Stream *Client::openInput(const DeviceInfo::Id &id, QIODevice *source, const Format &format)
{
    auto targetPlugin = pluginForDevice(id);
    if (!targetPlugin) {
        return nullptr;
    }
    return targetPlugin->open(id, Plugin::Direction::Input, format, source);
}

} // namespace audio
