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
#include "measurementaudiothread.h"

MeasurementAudioThread::MeasurementAudioThread(QObject *parent):
    QThread(parent),
    m_audio(nullptr),
    m_iodevice(this)
{
    start();
    QObject::moveToThread(this);

    connect(this, SIGNAL(finished()), SLOT(stop()));
    connect(&m_iodevice, SIGNAL(recived(const char *, qint64)), SIGNAL(recived(const char *, qint64)));
}
void MeasurementAudioThread::stop()
{
    if (m_audio) {
        m_audio->stop();
    }
    m_iodevice.close();
}
void MeasurementAudioThread::selectDevice(QAudioDeviceInfo deviceInfo, bool restart)
{
    if (m_audio) {
        m_audio->stop();
        delete m_audio;
    }
    m_iodevice.close();
    m_maxChanelCount = 0;

    m_device = deviceInfo;

    m_chanelCount = std::max(m_dataChanel, m_referenceChanel) + 1;
    foreach (auto c, m_device.supportedChannelCounts()) {
        unsigned int formatChanels = static_cast<unsigned int>(c);
        if (formatChanels > m_chanelCount)
            m_chanelCount = formatChanels;
        m_maxChanelCount = std::max(formatChanels, m_maxChanelCount);
    }

    m_format.setSampleRate(48000);
    m_format.setChannelCount(static_cast<int>(m_chanelCount));
    m_format.setSampleSize(32);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::Float);

    m_audio = new QAudioInput(m_device, m_format, this);
    m_iodevice.open(InputDevice::WriteOnly);
    emit deviceChanged();

    if (restart) {
        m_audio->start(&m_iodevice);
        m_format = m_audio->format();
        m_chanelCount = static_cast<unsigned int>(m_format.channelCount());

        emit formatChanged();
    }
}
void MeasurementAudioThread::setActive(bool active)
{
    if (active && (
                m_audio->state() == QAudio::IdleState ||
                m_audio->state() == QAudio::StoppedState)
       ) {
        m_audio->start(&m_iodevice);
        return ;
    }

    if (!active && m_audio->state() == QAudio::ActiveState) {
        m_audio->stop();
    }
}
