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
#include "deviceinfo.h"

namespace audio {

DeviceInfo::DeviceInfo(const Id &id, const QString &pluginName) :
    m_id(id), m_name(),
    m_pluginName(pluginName),
    m_inputChannels(), m_outputChannels()
{
}

QString DeviceInfo::name() const
{
    return m_name;
}

void DeviceInfo::setName(const QString &value)
{
    m_name = value;
}

QStringList DeviceInfo::inputChannels() const
{
    return m_inputChannels;
}

void DeviceInfo::setInputChannels(const QStringList &value)
{
    m_inputChannels = value;
}

QStringList DeviceInfo::outputChannels() const
{
    return m_outputChannels;
}

void DeviceInfo::setOutputChannels(const QStringList &value)
{
    m_outputChannels = value;
}

QString DeviceInfo::pluginName() const
{
    return m_pluginName;
}

DeviceInfo::Id DeviceInfo::id() const
{
    return m_id;
}

} // namespace audio
