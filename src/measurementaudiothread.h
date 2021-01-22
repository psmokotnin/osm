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
#ifndef MEASUREMENTAUDIOTHREAD_H
#define MEASUREMENTAUDIOTHREAD_H

#include <QThread>
#include <QDebug>
#include <QAudioInput>

class MeasurementAudioThread : public QThread
{
    Q_OBJECT

private:
    QAudioInput *m_audio;

    QAudioDeviceInfo m_device;
    QAudioFormat m_format;

    unsigned int m_chanelCount = 2,   //how many chanels will be presented in writeData
                 m_maxChanelCount = 2,//max supported channels by selected device
                 m_dataChanel = 0,
                 m_referenceChanel = 1
                                     ;
    int m_sampleRate;
    bool m_try = false;

    void startAudio();

public:
    explicit MeasurementAudioThread(QObject *parent);

    QAudioFormat format() const
    {
        return m_format;
    }
    QAudioDeviceInfo device() const
    {
        return m_device;
    }
    unsigned int chanelsCount() const
    {
        return m_maxChanelCount;
    }

    unsigned int referenceChanel() const
    {
        return m_referenceChanel;
    }
    void setReferenceChanel(unsigned int n)
    {
        m_referenceChanel = n;
    }

    unsigned int dataChanel() const
    {
        return m_dataChanel;
    }
    void setDataChanel(unsigned int n)
    {
        m_dataChanel = n;
    }

    int bufferSize() const
    {
        return m_audio->bufferSize();
    }
    int bytesReady() const
    {
        return  m_audio->bytesReady();
    }

    int sampleRate() const noexcept
    {
        return m_sampleRate;
    }

public slots:
    void setActive(bool active);
    void stop();
    void selectDevice(const QAudioDeviceInfo &deviceInfo, bool restart);
    void audioStateChanged(QAudio::State state);

signals:
    void deviceChanged(QString);
    void recived(const QByteArray &);
    void formatChanged();
    void started();
    void stopped();
    void deviceError();
};

#endif // MEASUREMENTAUDIOTHREAD_H
