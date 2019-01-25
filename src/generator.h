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

#include "generatorthread.h"

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
    Q_PROPERTY(int frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)

    //Gain
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)

private:
    GeneratorThread m_thread;

public:
    explicit Generator(QObject* parent = nullptr);
    ~Generator();

    bool getEnabled() {return m_thread.enabled();}
    void setEnabled(bool enable);

    int getType() {return m_thread.type();}
    void setType(int t);

    QVariant getAvailableTypes(void) {return m_thread.getAvailableTypes();}
    QVariant getDeviceList(void) {return m_thread.getDeviceList();}

    QString deviceName() {return m_thread.deviceName();}
    void selectDevice(QString name);

    int frequency() {return m_thread.frequency();}
    void setFrequency(int f);

    int gain() {return m_thread.gain();}
    void setGain(float gain);

signals:
    void enabledChanged(bool);
    void typeChanged();
    void frequencyChanged(int f);
    void deviceChanged();
    void gainChanged(float);

public slots:
};

#endif // GENERATOR_H
