/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QDebug>

#include "generatorthread.h"
#include "sinsweep.h"
#include "common/settings.h"
#include "audio/deviceinfo.h"

class Generator : public QObject
{
    Q_OBJECT

    //On-off property
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled NOTIFY enabledChanged)

    //current type
    Q_PROPERTY(int type READ getType WRITE setType NOTIFY typeChanged)

    //Available generator types. Constant list.
    Q_PROPERTY(QVariant types READ getAvailableTypes CONSTANT)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)

    //Frequency
    Q_PROPERTY(int frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(int startFrequency READ startFrequency WRITE setStartFrequency NOTIFY
               startFrequencyChanged)
    Q_PROPERTY(int endFrequency READ endFrequency WRITE setEndFrequency NOTIFY endFrequencyChanged)

    //Gain
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(float duration READ duration WRITE setDuration NOTIFY durationChanged)

    Q_PROPERTY(int channel READ channel WRITE setChannel NOTIFY channelChanged)
    Q_PROPERTY(int aux READ aux WRITE setAux NOTIFY auxChanged)

private:
    GeneratorThread m_thread;
    Settings *m_settings;

    void loadSettings();

public:
    explicit Generator(Settings *settings, QObject *parent = nullptr);
    ~Generator();

    bool getEnabled() const
    {
        return m_thread.enabled();
    }
    void setEnabled(bool enable);

    int getType() const
    {
        return m_thread.type();
    }
    void setType(int t);

    QVariant getAvailableTypes(void) const
    {
        return m_thread.getAvailableTypes();
    }

    int frequency() const
    {
        return m_thread.frequency();
    }
    void setFrequency(int f);

    int startFrequency() const
    {
        return m_thread.startFrequency();
    }
    void setStartFrequency(int f);

    int endFrequency() const
    {
        return m_thread.endFrequency();
    }
    void setEndFrequency(int f);

    float gain() const
    {
        return m_thread.gain();
    }
    void setGain(float gain);

    int channelsCount() const
    {
        return m_thread.channelsCount();
    }
    int channel() const
    {
        return m_thread.channel();
    }
    void setChannel(int channel);
    int aux() const
    {
        return m_thread.aux();
    }
    void setAux(int channel);

    float duration() const;
    void setDuration(float duration);

    audio::DeviceInfo::Id deviceId() const;
    void setDeviceId(const audio::DeviceInfo::Id &deviceId);

signals:
    void enabledChanged(bool);
    void typeChanged();
    void frequencyChanged(int f);
    void startFrequencyChanged(int f);
    void endFrequencyChanged(int f);
    void gainChanged(float);
    void channelChanged(int);
    void auxChanged(int);
    void durationChanged(float);
    void deviceIdChanged(audio::DeviceInfo::Id);
};

#endif // GENERATOR_H
