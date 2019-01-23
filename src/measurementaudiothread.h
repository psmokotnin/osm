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

#include "inputdevice.h"

class MeasurementAudioThread : public QThread
{
    Q_OBJECT

private:
    QAudioInput* m_audio;

    InputDevice m_iodevice;
    QAudioDeviceInfo m_device;
    QAudioFormat m_format;

    unsigned int
        m_chanelCount = 2,   //how many chanels will be presented in writeData
        m_maxChanelCount = 2,//max supported channels by selected device
        m_dataChanel = 0,
        m_referenceChanel = 1
    ;

public:
    explicit MeasurementAudioThread(QObject *parent);

    QAudioFormat format() const {return m_format;}
    QAudioDeviceInfo device() const {return m_device;}
    unsigned int chanelsCount() const {return m_maxChanelCount;}

    unsigned int referenceChanel() const {return m_referenceChanel;}
    void setReferenceChanel(unsigned int n) {m_referenceChanel = n;}

    unsigned int dataChanel() const {return m_dataChanel;}
    void setDataChanel(unsigned int n) {m_dataChanel = n;}

public slots:
    void setActive(bool active);
    void stop();
    void selectDevice(QAudioDeviceInfo deviceInfo, bool restart);

signals:
    void deviceChanged();
    void recived(const char *data, qint64 len);
    void formatChanged();
};

#endif // MEASUREMENTAUDIOTHREAD_H
