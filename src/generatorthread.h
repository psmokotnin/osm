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
#include <QAudioOutput>

#include "outputdevice.h"

class GeneratorThread : public QThread
{
    Q_OBJECT

public:
    explicit GeneratorThread(QObject *parent);

private:
    QAudioOutput* m_audio;
    QAudioFormat m_format;
    QAudioDeviceInfo m_device;
    QList<OutputDevice*> m_sources;

    int m_type;
    int m_frequency;
    bool m_enabled;

    void _selectDevice(QAudioDeviceInfo device);
    void _updateAudio();

public slots:
    void init();
    void finish();

    bool enabled() {return m_enabled;}
    void setEnabled(bool enable);

    int type() {return m_type;}
    void setType(int type);

    QVariant getDeviceList(void);
    void selectDevice(QString name);

    QString deviceName();
    QVariant getAvailableTypes(void);

    int frequency() {return m_frequency;}
    void setFrequency(int frequency);

signals:
    void enabledChanged(bool);
    void deviceChanged();
    void typeChanged();
    void frequencyChanged(int f);
};

#endif // GENERATORTHREAD_H
