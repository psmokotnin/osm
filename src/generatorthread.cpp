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
#include "generatorthread.h"
#include "QDebug"

#include "whitenoise.h"
#include "pinknoise.h"
#include "sinnoise.h"
#include "sinsweep.h"

GeneratorThread::GeneratorThread(QObject *parent) :
    QThread(parent),
    m_audio(nullptr),
    m_format(), m_device(), m_sources(),
    m_gain(-6.f), m_duration(1.f),
    m_type(0),
    m_frequency(1000),
    m_startFrequency(20),
    m_endFrequency(20000),
    m_channelCount(1),
    m_channel(0),
    m_aux(1),
    m_enabled(false)
{
    start();
    QObject::moveToThread(this);
}

float GeneratorThread::duration() const
{
    return m_duration;
}

void GeneratorThread::setDuration(float duration)
{
    if (!qFuzzyCompare(duration, m_duration)) {
        m_duration = duration;
        emit durationChanged(m_duration);
    }
}

void GeneratorThread::init()
{
    m_sources << new PinkNoise(this);
    m_sources << new WhiteNoise(this);
    m_sources << new SinNoise(this);
    m_sources << new SinSweep(this);
    m_device = QAudioDeviceInfo::defaultOutputDevice();
    _selectDevice(m_device);
    connect(this, SIGNAL(finished()), this, SLOT(finish()));
}
/*
 * finishes audio in current thread
 */
void GeneratorThread::finish()
{
    setEnabled(false);
}
QString GeneratorThread::deviceName() const
{
    return m_device.deviceName();
}
void GeneratorThread::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        _updateAudio();
        if(m_type == 3 && m_enabled == false)
            qobject_cast<SinSweep*>(m_sources[3])->setFrequency(m_startFrequency);
        emit enabledChanged(m_enabled);
    }
}
void GeneratorThread::setType(int type)
{
    if (m_type != type) {
        m_type = type;
        _updateAudio();
        emit typeChanged(m_type);
    }
}
void GeneratorThread::selectDevice(const QString &name)
{
    if (name == deviceName())
        return;

    QStringList devices = getDeviceList().value<QStringList>();
    if (devices.indexOf(name) == -1) {
        return;
    }
    foreach (auto &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        if (name == deviceInfo.deviceName()) {
            _selectDevice(deviceInfo);
        }
    }
}
void GeneratorThread::_selectDevice(const QAudioDeviceInfo &device)
{
    m_device = device;
    m_sources[m_type]->close();
    _updateAudio();
    emit deviceChanged(m_device.deviceName());
}
void GeneratorThread::_updateAudio()
{
    if (m_audio) {
        m_audio->stop();
        delete m_audio;
        m_audio = nullptr;
    }


    m_channelCount = 1;
    foreach (auto formatChanels, m_device.supportedChannelCounts()) {
        if (formatChanels > m_channelCount)
            m_channelCount = formatChanels;
    }
    m_format = m_device.preferredFormat();
    m_format.setSampleSize(32);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::Float);
    m_format.setChannelCount(m_channelCount);
    if (m_format.sampleRate() < 44100) {
        m_format.setSampleRate(44100);
    }
    if (!m_device.isFormatSupported(m_format)) {
        m_format = m_device.nearestFormat(m_format);
    }
    if (m_enabled) {

        m_audio = new QAudioOutput(m_device, m_format, this);
#ifndef WIN64
        m_audio->setBufferSize(
                    static_cast<int>(sizeof(float)) *
                    static_cast<int>(m_channelCount) *
                    8*1024);
#endif

        if (m_sources[m_type]->openMode() == QIODevice::NotOpen) {
            m_sources[m_type]->open(QIODevice::ReadOnly);
        }
        m_sources[m_type]->setGain(m_gain);
        m_sources[m_type]->setChanel(m_channel);
        m_sources[m_type]->setAux(m_aux);
        m_sources[m_type]->setChanelCount(m_channelCount);
        m_sources[m_type]->setSamplerate(m_format.sampleRate());
        m_audio->start(m_sources[m_type]);

        emit channelsCountChanged();
    }
}
QVariant GeneratorThread::getAvailableTypes() const
{
    QStringList nameList;
    foreach (OutputDevice* o, m_sources) {
        nameList << o->name();
    }
    return QVariant::fromValue(nameList);
}
QVariant GeneratorThread::getDeviceList() const
{
    QStringList deviceList;
    QAudioFormat format44, format48;
    format48.setChannelCount(2);
    format48.setSampleRate(48000);
    format48.setSampleSize(32);
    format48.setCodec("audio/pcm");
    format48.setByteOrder(QAudioFormat::LittleEndian);
    format48.setSampleType(QAudioFormat::Float);

    format44 = format48;
    format44.setSampleRate(44100);

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        if (deviceInfo.isFormatSupported(format44) || deviceInfo.isFormatSupported(format48)) {
            deviceList << deviceInfo.deviceName();
        }
    }
    return QVariant::fromValue(deviceList);
}
void GeneratorThread::setFrequency(int frequency)
{
    if (m_frequency != frequency) {
        m_frequency = frequency;
        emit frequencyChanged(m_frequency);
    }
}
void GeneratorThread::setStartFrequency(int startFrequency)
{
    if(m_startFrequency != startFrequency)
    {
        m_startFrequency = startFrequency;
        emit startFrequencyChanged(m_startFrequency);
    }
}
void GeneratorThread::setEndFrequency(int endFrequency)
{
    if(m_endFrequency != endFrequency)
    {
        m_endFrequency = endFrequency;
        emit endFrequencyChanged(m_endFrequency);
    }
}
void GeneratorThread::setGain(float gain)
{
    if (!qFuzzyCompare(gain, m_gain)) {
        m_gain = gain;
        emit gainChanged(gain);
    }
}
void GeneratorThread::setChannel(int chanel)
{
    if (m_channel != chanel) {
        m_channel = chanel;
        m_sources[m_type]->setChanel(m_channel);
        emit channelChanged(m_channel);
    }
}
void GeneratorThread::setAux(int chanel)
{
    if (m_aux != chanel) {
        m_aux = chanel;
        m_sources[m_type]->setAux(m_aux);
        emit auxChanged(m_aux);
    }
}
