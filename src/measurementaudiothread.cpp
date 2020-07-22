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
    m_sampleRate(48000)
{
    start();
    QObject::moveToThread(this);

    connect(this, SIGNAL(finished()), SLOT(stop()));
}
void MeasurementAudioThread::stop()
{
    if (m_audio) {
        m_audio->stop();
    }
}
void MeasurementAudioThread::selectDevice(const QAudioDeviceInfo &deviceInfo, bool restart)
{
    if (m_audio) {
        m_audio->stop();
        m_audio->deleteLater();
        m_audio = nullptr;
    }
    m_maxChanelCount = 0;
    m_device = deviceInfo;
    emit deviceChanged(m_device.deviceName());

    if (restart) {
        startAudio();
    }
}
void MeasurementAudioThread::setActive(bool active)
{
    if (m_device.isNull()) {
        emit deviceError();
        return;
    }
    if (active) {
        return startAudio();
    }

    if (!active && m_audio && m_audio->state() != QAudio::StoppedState) {
        m_audio->stop();
    }
}
void MeasurementAudioThread::startAudio()
{
    m_chanelCount = std::max(m_dataChanel, m_referenceChanel) + 1;
    foreach (auto c, m_device.supportedChannelCounts()) {
        auto formatChanels = static_cast<unsigned int>(c);
        if (formatChanels > m_chanelCount)
            m_chanelCount = formatChanels;
        m_maxChanelCount = std::max(formatChanels, m_maxChanelCount);
    }

    m_format = m_device.preferredFormat();  //automatic sample rate
    if (m_format.sampleRate() < 44100) {
        m_format.setSampleRate(44100);
    }
    m_format.setChannelCount(static_cast<int>(m_chanelCount));
    m_format.setSampleSize(32);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::Float);
    if (!m_device.isFormatSupported(m_format)) {
        m_format = m_device.nearestFormat(m_format);
    }

    if (m_audio)
        m_audio->deleteLater();

    m_audio = new QAudioInput(m_device, m_format, this);
#ifndef WIN64
    m_audio->setBufferSize(
                static_cast<int>(sizeof(float)) *
                static_cast<int>(m_chanelCount) *
                8*1024);
#endif
    connect(m_audio, &QAudioInput::stateChanged, this, &MeasurementAudioThread::audioStateChanged);

    auto io = m_audio->start();
    if (m_audio->error() == QAudio::OpenError) {
        emit deviceError();
        return;
    }
    connect(io, &QIODevice::readyRead,
                [&, io]() {
                    int len = m_audio->bytesReady();
                    QByteArray buffer(len, 0x1);
                    qint64 l;

                    while ((l = io->read(buffer.data(), len)) && l > 0) {
                        emit recived(buffer);
                    }
        });
    m_format = m_audio->format();
    m_sampleRate = m_format.sampleRate();
    m_chanelCount = static_cast<unsigned int>(m_format.channelCount());
    emit formatChanged();
}
void MeasurementAudioThread::audioStateChanged(QAudio::State state)
{
    if (m_audio && m_audio->error() != QAudio::NoError) {
        if (!m_try) {
            m_try = true;
            startAudio();
        } else {
            emit deviceError();
            m_audio->stop();
        }
    }

    switch (state) {
        case QAudio::ActiveState:
            emit started();
            m_try = false;
            break;

        case QAudio::IdleState:
        case QAudio::StoppedState:
            emit stopped();
            break;

        case QAudio::SuspendedState:
        case QAudio::InterruptedState:
            break;
    }
}
