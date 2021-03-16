/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#include "inputdevice.h"

InputDevice::InputDevice(QObject *parent) : QIODevice(parent)
{
}

qint64 InputDevice::writeData(const char *data, qint64 len)
{
    if (m_callback) {
        QByteArray buffer(data, len);
        m_callback(buffer);
    }
    return len;
}

qint64 InputDevice::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);

    return -1;
}

void InputDevice::setCallback(const std::function<void(const QByteArray &buffer)> &callback)
{
    m_callback = callback;
}
