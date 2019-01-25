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
#include <math.h>

OutputDevice::OutputDevice(QObject *parent) : QIODevice(parent),
    m_chanel(1),
    m_chanelCount(1),
    m_gain(1.f)
{
    connect(parent, SIGNAL(gainChanged(float)), this, SLOT(setGain(float)));
}
qint64 OutputDevice::readData(char *data, qint64 maxlen)
{
    qint64 total = 0;
    int chanel = m_chanelCount;
    const float zero = 0.f;
    Sample src;

    while (maxlen - total > 0) {
        if (chanel >= m_chanelCount) {
            chanel = 0;
            src = this->sample();
        }

        if (chanel == m_chanel || chanel == m_aux) {
            memcpy(data + total, &src.f, sizeof(float));
        } else {
            memcpy(data + total, &zero, sizeof(float));
        }
        total += sizeof(float);

        ++chanel;
    }
    return total;
}
Sample OutputDevice::sample(void)
{
    Sample output;
    output.f = 0.0;
    return output;
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
