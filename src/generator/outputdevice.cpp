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
#include "outputdevice.h"
#include <cmath>
#include <cstring>
#include "generatorthread.h"

OutputDevice::OutputDevice(QObject *parent) : QIODevice(parent),
    m_name("Silent"),
    m_sampleRate(0),
    m_chanelCount(1),
    m_gain(1.f)
{
    connect(parent, SIGNAL(gainChanged(float)), this, SLOT(setGain(float)));
}

OutputDevice::~OutputDevice()
{
    close();
}
qint64 OutputDevice::readData(char *data, qint64 maxlen)
{
    qint64 total = 0;
    int chanel = m_chanelCount;
    Sample src = {0.f}, inv = src;
    std::memset(data, 0, maxlen);
    auto generator = static_cast<GeneratorThread *>(parent());
    if (generator) {
        m_channels = generator->channels();
    }

    while (maxlen - total > 0) {
        if (chanel >= m_chanelCount) {
            chanel = 0;
            src = this->sample();
            inv.f = -src.f;
            emit sampleOut(src.f);
            if (std::isnan(src.f)) {
                emit sampleError();
                return 0;
            }
        }

        if (m_channels.contains(chanel)) {
            memcpy(data + total, (chanel % 2 && generator->evenPolarity() ? &inv.f : &src.f), sizeof(float));
        }

        total += sizeof(float);
        ++chanel;
    }
    return total;
}
Sample OutputDevice::sample()
{
    Sample output = {0.f};
    return output;
}

QString OutputDevice::name() const
{
    return m_name;
}

void OutputDevice::close()
{
    QIODevice::close();
}

void OutputDevice::setSamplerate(int sampleRate)
{
    m_sampleRate = sampleRate;
}
qint64 OutputDevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return -1;
}
void OutputDevice::setGain(float gaindB)
{
    m_gain = powf(10.f, gaindB / 20.f);
}

void OutputDevice::setChanelCount(int count)
{
    m_chanelCount = count;
}
