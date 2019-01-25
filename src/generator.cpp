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

Generator::Generator(QObject *parent) : QObject(parent),
    m_thread(parent)
{
    QMetaObject::invokeMethod(&m_thread, "init", Qt::QueuedConnection);

    connect(&m_thread, SIGNAL(enabledChanged(bool)), this, SIGNAL(enabledChanged(bool)), Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(deviceChanged()), this, SIGNAL(deviceChanged()), Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(typeChanged()), this, SIGNAL(typeChanged()), Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(frequencyChanged(int)), this, SIGNAL(frequencyChanged(int)), Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(gainChanged(float)), this, SIGNAL(gainChanged(float)), Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(chanelChanged(int)), this, SIGNAL(chanelChanged(int)), Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(auxChanged(int)), this, SIGNAL(auxChanged(int)), Qt::QueuedConnection);
    connect(&m_thread, SIGNAL(chanelsCountChanged()), this, SIGNAL(chanelsCountChanged()), Qt::QueuedConnection);
}
Generator::~Generator()
{
    m_thread.quit();
    m_thread.wait();
}
void Generator::setEnabled(bool enabled)
{
    QMetaObject::invokeMethod(
                &m_thread,
                "setEnabled",
                Qt::QueuedConnection,
                Q_ARG(bool, enabled)
    );
}
void Generator::setType(int type)
{
    QMetaObject::invokeMethod(
                &m_thread,
                "setType",
                Qt::QueuedConnection,
                Q_ARG(int, type)
    );
}
void Generator::selectDevice(QString name)
{
    QMetaObject::invokeMethod(
                &m_thread,
                "selectDevice",
                Qt::QueuedConnection,
                Q_ARG(QString, name)
    );
}
void Generator::setFrequency(int frequency)
{
    QMetaObject::invokeMethod(
                &m_thread,
                "setFrequency",
                Qt::QueuedConnection,
                Q_ARG(int, frequency)
    );
}
void Generator::setGain(float gain)
{
    QMetaObject::invokeMethod(
                &m_thread,
                "setGain",
                Qt::QueuedConnection,
                Q_ARG(float, gain)
    );
}
void Generator::setChanel(int chanel)
{
    QMetaObject::invokeMethod(
                &m_thread,
                "setChanel",
                Qt::QueuedConnection,
                Q_ARG(int, chanel)
    );
}
void Generator::setAux(int chanel)
{
    QMetaObject::invokeMethod(
                &m_thread,
                "setAux",
                Qt::QueuedConnection,
                Q_ARG(int, chanel)
    );
}
