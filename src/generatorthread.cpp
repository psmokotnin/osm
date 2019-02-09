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

GeneratorThread::GeneratorThread(QObject *parent) :
    QThread(parent),
    m_audio(nullptr),
    m_format(), m_device(), m_sources(),
    m_gain(-6.f),
    m_type(0),
    m_frequency(1000),
    m_chanelCount(1),
    m_chanel(0),
    m_aux(1),
    m_enabled(false)
{
    start();
    QObject::moveToThread(this);
}
void GeneratorThread::init()
{
    m_sources << new PinkNoise(this);
    m_sources << new WhiteNoise(this);
    m_sources << new SinNoise(this);

    m_device = QAudioDeviceInfo::defaultOutputDevice();
    m_format.setSampleRate(48000);
    m_format.setSampleSize(32);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::Float);

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
        emit enabledChanged(m_enabled);
    }
}
void GeneratorThread::setType(int type)
{
    if (m_type != type) {
        m_type = type;
        _updateAudio();
        emit typeChanged();
    }
}
void GeneratorThread::selectDevice(const QString &name)
{
    if (name == deviceName())
        return;

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

    if (m_audio) {
        m_audio->stop();
        delete m_audio;
    }
    m_chanelCount = 1;
    foreach (auto formatChanels, m_device.supportedChannelCounts()) {
        if (formatChanels > m_chanelCount)
            m_chanelCount = formatChanels;
    }
    m_format.setChannelCount(m_chanelCount);
    m_audio = new QAudioOutput(m_device, m_format, this);
    m_audio->setBufferSize(
                static_cast<int>(sizeof(float)) *
                static_cast<int>(m_chanelCount) *
                8*1024);

    _updateAudio();
    emit deviceChanged();
    emit chanelsCountChanged();
}
void GeneratorThread::_updateAudio()
{
    if (m_enabled) {
        if (m_sources[m_type]->openMode() == QIODevice::NotOpen) {
            m_sources[m_type]->open(QIODevice::ReadOnly);
        }
        m_sources[m_type]->setGain(m_gain);
        m_sources[m_type]->setChanel(m_chanel);
        m_sources[m_type]->setAux(m_aux);
        m_sources[m_type]->setChanelCount(m_chanelCount);
        m_sources[m_type]->setSamplerate(m_format.sampleRate());
        m_audio->start(m_sources[m_type]);
    } else {
        m_audio->stop();
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
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        deviceList << deviceInfo.deviceName();
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
void GeneratorThread::setGain(float gain)
{
    if (!qFuzzyCompare(gain, m_gain)) {
        m_gain = gain;
        emit gainChanged(gain);
    }
}
void GeneratorThread::setChanel(int chanel)
{
    if (m_chanel != chanel) {
        m_chanel = chanel;
        m_sources[m_type]->setChanel(m_chanel);
        emit chanelChanged(m_chanel);
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
