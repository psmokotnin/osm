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

Generator::Generator(QObject *parent) : QObject(parent)
{
    _sources << new PinkNoise(this);
    _sources << new WhiteNoise(this);
    _sources << new SinNoise(this);

    _device = QAudioDeviceInfo::defaultOutputDevice();
    // Set up the format, eg.
    _format.setSampleRate(48000);
    _format.setChannelCount(1);
    _format.setSampleSize(32);
    _format.setCodec("audio/pcm");
    _format.setByteOrder(QAudioFormat::LittleEndian);
    _format.setSampleType(QAudioFormat::Float);

    _audio = new QAudioOutput(_device, _format, this);
    _audio->setBufferSize(16384);
//    _audio->setVolume(m_gain);
    //connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
}
bool Generator::getEnabled()
{
    return enabled;
}
void Generator::setEnabled(bool enable)
{
    if (enabled != enable) {
        enabled = enable;
        updateAudio();
    }
}
int Generator::getType()
{
    return type;
}
void Generator::setType(int t)
{
    type = t;

    updateAudio();
}
void Generator::updateAudio(void)
{
    if (enabled) {
        if (_sources[type]->openMode() == QIODevice::NotOpen)
            _sources[type]->open(QIODevice::ReadOnly);

        _sources[type]->setSamplerate(_format.sampleRate());
        _audio->setVolume(m_gain);
//        qDebug() << m_gain;
        _audio->start(_sources[type]);
    }
    else
        _audio->stop();
}

QVariant Generator::getAvailableTypes(void)
{
    QStringList nameList;
    foreach (OutputDevice* o, _sources) {
        nameList << o->name;
    }
    return QVariant::fromValue(nameList);
}
QVariant Generator::getDeviceList(void)
{
    QStringList deviceList;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        deviceList << deviceInfo.deviceName();
    }
    return QVariant::fromValue(deviceList);
}
QString Generator::deviceName()
{
    return _device.deviceName();
}
void Generator::selectDevice(QString name)
{
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        if (name == deviceInfo.deviceName()) {
            _device = deviceInfo;

            _audio->stop();
            _sources[type]->close();
            delete _audio;
            _audio = new QAudioOutput(_device, _format, this);
            _audio->setBufferSize(16384);

            updateAudio();
//            if (enabled) {
//                _sources[type]->open(QIODevice::ReadOnly);
//                _sources[type]->setSamplerate(_format.sampleRate());
//                _audio->start(_sources[type]);
//            }
        }
    }
}
int Generator::getFrequency()
{
    return frequency;
}
void Generator::setFrequency(int f)
{
    frequency = f;
    emit frequencyChanged(frequency);
}

double Generator::gain() const
{
    return m_gain;
}

void Generator::setGain(double gain)
{
    if (qFuzzyCompare(m_gain, gain))
        return;

    m_gain = gain;
//    updateAudio();
    _audio->setVolume(m_gain);
    emit gainChanged(m_gain);
}
