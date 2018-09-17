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
#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QDebug>
#include <QAudio>
#include <QAudioDeviceInfo>
#include <QAudioOutput>

#include "whitenoise.h"
#include "pinknoise.h"
#include "sinnoise.h"

class Generator : public QObject
{
    Q_OBJECT

    //On-off property
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled NOTIFY enabledChanged)

    //current type
    Q_PROPERTY(int type READ getType WRITE setType NOTIFY typeChanged)

    //Available generator types. Constant list.
    Q_PROPERTY(QVariant types READ getAvailableTypes CONSTANT)

    //Available output devices
    Q_PROPERTY(QVariant devices READ getDeviceList CONSTANT)
    Q_PROPERTY(QString device READ deviceName WRITE selectDevice NOTIFY deviceChanged)

    //Frequency
    Q_PROPERTY(int frequency READ getFrequency WRITE setFrequency NOTIFY frequencyChangedQml)
    //Gain
    Q_PROPERTY(double gain READ gain WRITE setGain NOTIFY gainChanged)

private:
    bool enabled    = false;
    int type        = 0;
    int frequency   = 1000;
    double m_gain   = 0.2;

    QAudioFormat _format;
    QAudioOutput* _audio;
    QAudioDeviceInfo _device;
    QList<OutputDevice*> _sources;

protected:
    void updateAudio(void);

public:
    explicit Generator(QObject* parent = nullptr);

    bool getEnabled();
    void setEnabled(bool enable);

    int getType();
    void setType(int t);

    QVariant getAvailableTypes(void);
    QVariant getDeviceList(void);

    QString deviceName();
    void selectDevice(QString name);

    int getFrequency();
    void setFrequency(int f);

    double gain() const;

signals:
    void enabledChanged();
    void typeChanged();
    void frequencyChanged(int f);
    void frequencyChangedQml();
    void deviceChanged();

    void gainChanged(double gain);

public slots:
void setGain(double gain);
};

#endif // GENERATOR_H
