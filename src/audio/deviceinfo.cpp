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
#include <QDebug>
namespace audio {

DeviceInfo::DeviceInfo() :
    m_id(), m_name(),
    m_defaultSampleRate(1),
    m_pluginName(),
    m_inputChannels(), m_outputChannels()
{
}

DeviceInfo::DeviceInfo(const Id &id, const QString &pluginName) :
    m_id(id), m_name(), m_defaultSampleRate(1),
    m_pluginName(pluginName),
    m_inputChannels(), m_outputChannels()
{
}

bool DeviceInfo::operator==(const DeviceInfo &right) const
{
    return id() == right.id();
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

unsigned int DeviceInfo::defaultSampleRate() const
{
    return m_defaultSampleRate;
}

void DeviceInfo::setDefaultSampleRate(unsigned int defaultSampleRate)
{
    m_defaultSampleRate = defaultSampleRate;
}

DeviceInfo::Id DeviceInfo::id() const
{
    return m_id;
}

} // namespace audio

QDebug operator << (QDebug dbg, const audio::DeviceInfo &info)
{
    dbg.nospace() << "Audio device: {:"
                  << info.pluginName() << info.id() << " name:" << info.name() << info.defaultSampleRate() << "}";

    return dbg.maybeSpace();
}
