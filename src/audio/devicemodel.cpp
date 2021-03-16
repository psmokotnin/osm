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
#include "devicemodel.h"
#include "client.h"

namespace audio {

DeviceModel::DeviceModel(QObject *parent) : QAbstractListModel(parent), m_client(Client::getInstance()),
    m_scope(InputOnly),
    m_list()
{
    updateLists();
    qRegisterMetaType<audio::DeviceInfo::Id>("audio::DeviceInfo::Id");
    connect(this, &DeviceModel::scopeChanged, this, &DeviceModel::updateLists);
    connect(m_client.get(), &Client::deviceListChanged, this, &DeviceModel::updateLists);
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable;
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_list.empty())
        return 0;
    return m_list.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_list.empty())
        return QVariant();

    QVariant r;
    switch (role) {
    case NameRole:
        r.setValue(m_list[index.row()].name());
        break;

    case IdRole:
        r.setValue(m_list[index.row()].id());
        break;
    }

    return r;
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[IdRole]   = "deviceID";
    names[NameRole] = "name";
    return names;
}

void DeviceModel::updateLists()
{
    beginResetModel();
    m_list = {};
    for (auto &deviceInfo : m_client->getDeviceList()) {
        switch (m_scope) {
        case DeviceScope::InputOnly:
            if (deviceInfo.inputChannels().size() > 0) {
                m_list.push_back(deviceInfo);
            }
            break;
        case DeviceScope::OutputOnly:
            if (deviceInfo.outputChannels().size() > 0) {
                m_list.push_back(deviceInfo);
            }
            break;
        }
    }
    endResetModel();
}

DeviceModel::DeviceScope DeviceModel::scope() const
{
    return m_scope;
}

void DeviceModel::setScope(const DeviceScope &scope)
{
    if (m_scope != scope) {
        m_scope = scope;
        emit scopeChanged();
    }
}

DeviceInfo::Id DeviceModel::deviceId(const int &index)
{
    if (index < 0 || index > m_list.size() || m_list.empty())
        return {};

    auto &deviceInfo = m_list[index];
    return deviceInfo.id();
}

int DeviceModel::indexOf(const DeviceInfo::Id &deviceId)
{

    auto find = std::find_if(m_list.begin(), m_list.end(), [&deviceId](auto element) {
        return element.id() == deviceId;
    });

    if (find != m_list.end()) {
        return std::distance(m_list.begin(), find);
    }
    return -1;
}

QStringList DeviceModel::channelNames(const int &index)
{
    if (index < 0 || index > m_list.size() || m_list.empty())
        return {};

    auto &deviceInfo = m_list[index];
    switch (m_scope) {
    case DeviceScope::InputOnly:
        return deviceInfo.inputChannels();
        break;
    case DeviceScope::OutputOnly:
        return deviceInfo.outputChannels();
        break;
    }
    return {};
}

} // namespace audio
