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
#include "generator.h"

Generator::Generator(Settings *settings, QObject *parent) : QObject(parent),
    m_thread(parent), m_settings(settings), m_uuid(QUuid::createUuid())
{
    QMetaObject::invokeMethod(&m_thread, "init", Qt::QueuedConnection);

    connect(&m_thread, SIGNAL(enabledChanged(bool)),  this, SIGNAL(enabledChanged(bool)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(evenPolarityChanged(bool)),  this, SIGNAL(evenPolarityChanged(bool)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(typeChanged(int)),      this, SIGNAL(typeChanged()),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(frequencyChanged(int)), this, SIGNAL(frequencyChanged(int)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(startFrequencyChanged(int)), this, SIGNAL(startFrequencyChanged(int)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(endFrequencyChanged(int)), this, SIGNAL(endFrequencyChanged(int)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(gainChanged(float)),    this, SIGNAL(gainChanged(float)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(durationChanged(float)), this, SIGNAL(durationChanged(float)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(channelsChanged(QSet<int>)),   this, SIGNAL(channelsChanged(QSet<int>)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(deviceIdChanged(audio::DeviceInfo::Id)),  this,
            SIGNAL(deviceIdChanged(audio::DeviceInfo::Id)),
            Qt::QueuedConnection);
    connect(&m_thread, &GeneratorThread::deviceError, this, [this]() {
        setEnabled(false);
    });
    connect(&m_thread, &GeneratorThread::channelsChanged, this, [this](QSet<int> set) {
        QList<QVariant> channelsList = {};

        channelsList.reserve(set.count());
        for (auto &channel : set) {
            channelsList.append(channel);
        }
        emit channelsChanged(set);
        emit channelsChangedQList(channelsList);
    });

    loadSettings();
}
Generator::~Generator()
{
    m_thread.quit();
    m_thread.wait();
}
audio::DeviceInfo::Id Generator::deviceId() const
{
    return m_thread.deviceId();
}

void Generator::setDeviceId(const audio::DeviceInfo::Id &deviceId)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setDeviceId",
        Qt::QueuedConnection,
        Q_ARG(audio::DeviceInfo::Id, deviceId)
    );
}

void Generator::loadSettings()
{
    if (m_settings) {
        setType(m_settings->reactValue<GeneratorThread, int>(
                    "type", &m_thread, &GeneratorThread::typeChanged, m_thread.type()).toInt());

        setFrequency(m_settings->reactValue<GeneratorThread, int>(
                         "frequency", &m_thread, &GeneratorThread::frequencyChanged, m_thread.frequency()).toInt());

        setGain(m_settings->reactValue<GeneratorThread, float>(
                    "gain", &m_thread, &GeneratorThread::gainChanged, m_thread.gain()).toFloat());

        setDeviceId(m_settings->reactValue<GeneratorThread, audio::DeviceInfo::Id>(
                        "deviceId", &m_thread, &GeneratorThread::deviceIdChanged, m_thread.deviceId()).toString());

        setChannels(m_settings->reactValue<Generator, QList<QVariant>>(
                        "channels", this, &Generator::channelsChangedQList, QList<QVariant>()).toList());

        setEvenPolarity(m_settings->reactValue<GeneratorThread, bool>(
                            "evenPolarity", &m_thread, &GeneratorThread::evenPolarityChanged, m_thread.evenPolarity()).toBool());

        //@TODO: Add settings for SinSweep parameters
    }
}

QSet<int> Generator::channels() const
{
    return m_thread.channels();
}

void Generator::setChannels(const QSet<int> &channels)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setChannels",
        Qt::QueuedConnection,
        Q_ARG(QSet<int>, channels)
    );
}

void Generator::setChannels(const QList<QVariant> channels)
{
    QSet<int> set = {};
    for (auto &channel : channels) {
        bool ok = false;
        auto number = channel.toInt(&ok);
        if (ok) {
            set << number;
        }
    }
    setChannels(set);
}

bool Generator::evenPolarity() const
{
    return m_thread.evenPolarity();
}

void Generator::setEvenPolarity(bool newevenPolarity)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setEvenPolarity",
        Qt::QueuedConnection,
        Q_ARG(bool, newevenPolarity)
    );
}

QUuid Generator::uuid() const
{
    return m_uuid;
}

void Generator::setEnabled(bool enabled)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setEnabled",
        Qt::QueuedConnection,
        Q_ARG(bool, enabled)
    );
}
void Generator::setType(int type)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setType",
        Qt::QueuedConnection,
        Q_ARG(int, type)
    );
}
void Generator::setFrequency(int frequency)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setFrequency",
        Qt::QueuedConnection,
        Q_ARG(int, frequency)
    );
}

void Generator::setStartFrequency(int f)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setStartFrequency",
        Qt::QueuedConnection,
        Q_ARG(int, f)
    );
}
void Generator::setEndFrequency(int f)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setEndFrequency",
        Qt::QueuedConnection,
        Q_ARG(int, f)
    );
}
void Generator::setGain(float gain)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setGain",
        Qt::QueuedConnection,
        Q_ARG(float, gain)
    );
}

float Generator::duration() const
{
    return m_thread.duration();
}

void Generator::setDuration(float duration)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setDuration",
        Qt::QueuedConnection,
        Q_ARG(float, duration)
    );
}
