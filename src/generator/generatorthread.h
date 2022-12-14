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
#ifndef GENERATORTHREAD_H
#define GENERATORTHREAD_H

#include <QThread>

#include "outputdevice.h"
#include "sinsweep.h"
#include "audio/deviceinfo.h"
#include "audio/stream.h"

class GeneratorThread : public QThread
{
    Q_OBJECT

public:
    explicit GeneratorThread(QObject *parent);
    ~GeneratorThread();

    static GeneratorThread *getInstance();

public slots:
    void init();
    void finish();

    bool enabled() const
    {
        return m_enabled;
    }
    void setEnabled(bool enable);

    int type() const
    {
        return m_type;
    }
    void setType(int type);

    audio::DeviceInfo::Id deviceId() const;
    void setDeviceId(audio::DeviceInfo::Id deviceId);

    QVariant getAvailableTypes() const;

    int frequency() const
    {
        return m_frequency;
    }
    void setFrequency(int frequency);

    int startFrequency() const
    {
        return m_startFrequency;
    }
    void setStartFrequency(int startFrequency);

    int endFrequency() const
    {
        return m_endFrequency;
    }
    void setEndFrequency(int endFrequency);

    float gain() const
    {
        return m_gain;
    }
    void setGain(float gain);

    float duration() const;
    void setDuration(float duration);

    QSet<int> channels() const;
    void setChannels(const QSet<int> &channels);

    bool evenPolarity() const;
    void setEvenPolarity(bool evenPolarity);

signals:
    void enabledChanged(bool);
    void typeChanged(int);
    void frequencyChanged(int);
    void startFrequencyChanged(int);
    void endFrequencyChanged(int);
    void gainChanged(float);
    void durationChanged(float);
    void deviceIdChanged(audio::DeviceInfo::Id);
    void deviceError();
    void sampleOut(float);
    void channelsChanged(QSet<int>);

    void evenPolarityChanged(bool);

private:
    void updateAudio();

    static GeneratorThread *s_instance;
    audio::DeviceInfo::Id m_deviceId;
    audio::Stream *m_audioStream;
    QList<OutputDevice *> m_sources;
    QSet<int> m_channels;
    mutable std::mutex m_channelsMutex;

    float m_gain, m_duration;
    int m_type;
    int m_frequency, m_startFrequency, m_endFrequency;
    bool m_enabled, m_evenPolarity;
};

#endif // GENERATORTHREAD_H
