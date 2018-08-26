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

OutputDevice::OutputDevice(QObject *parent) : QIODevice(parent)
{

}
qint64 OutputDevice::readData(char *data, qint64 maxlen)
{
    qint64 total = 0;
    while (maxlen - total > 0) {
        const Sample src = this->sample();
        memcpy(data + total, &src.f, sizeof(float));
        total += sizeof(float);
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
