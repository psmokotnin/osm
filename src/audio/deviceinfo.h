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
#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

#include <QStringList>
#include <QMetaType>

namespace audio {

class DeviceInfo
{
public:
    using Id = QString;
    using List = QList<DeviceInfo>;

    DeviceInfo();
    DeviceInfo(const Id &id, const QString &pluginName);
    bool operator==(const DeviceInfo &right) const;

    Id id() const;

    QString name() const;
    void setName(const QString &value);

    QStringList inputChannels() const;
    void setInputChannels(const QStringList &value);

    QStringList outputChannels() const;
    void setOutputChannels(const QStringList &value);

    QString pluginName() const;

    unsigned int defaultSampleRate() const;
    void setDefaultSampleRate(unsigned int defaultSampleRate);

private:
    Id m_id;
    QString m_name;
    unsigned int m_defaultSampleRate;
    QString m_pluginName;
    QStringList m_inputChannels;
    QStringList m_outputChannels;
};

} // namespace audio
Q_DECLARE_METATYPE(audio::DeviceInfo::Id);
QDebug operator << (QDebug dbg, const audio::DeviceInfo &info);

#endif // AUDIO_DEVICE_H
