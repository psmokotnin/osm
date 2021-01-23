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
    m_thread(parent), m_settings(settings)
{
    QMetaObject::invokeMethod(&m_thread, "init", Qt::QueuedConnection);

    connect(&m_thread, SIGNAL(enabledChanged(bool)),  this, SIGNAL(enabledChanged(bool)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(deviceChanged(QString)), this, SIGNAL(deviceChanged()),
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
    connect(&m_thread, SIGNAL(channelChanged(int)),   this, SIGNAL(channelChanged(int)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(auxChanged(int)),       this, SIGNAL(auxChanged(int)),
            Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(channelsCountChanged()), this, SIGNAL(channelsCountChanged()),
            Qt::QueuedConnection);

    loadSettings();
}
Generator::~Generator()
{
    m_thread.quit();
    m_thread.wait();
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

        setChannel(m_settings->reactValue<GeneratorThread, int>(
                       "channel", &m_thread, &GeneratorThread::channelChanged, m_thread.channel()).toInt());
        setAux(m_settings->reactValue<GeneratorThread, int>(
                   "aux", &m_thread, &GeneratorThread::auxChanged, m_thread.aux()).toInt());

        selectDevice(m_settings->reactValue<GeneratorThread, QString>(
                         "device", &m_thread, &GeneratorThread::deviceChanged, m_thread.deviceName()).toString());
        //@TODO: Add settings for SinSweep parameters
    }
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
void Generator::selectDevice(const QString &name)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "selectDevice",
        Qt::QueuedConnection,
        Q_ARG(QString, name)
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
void Generator::setChannel(int channel)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setChannel",
        Qt::QueuedConnection,
        Q_ARG(int, channel)
    );
}
void Generator::setAux(int channel)
{
    QMetaObject::invokeMethod(
        &m_thread,
        "setAux",
        Qt::QueuedConnection,
        Q_ARG(int, channel)
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
