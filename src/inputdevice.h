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
#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <QIODevice>

class InputDevice : public QIODevice
{
    Q_OBJECT

public:
    InputDevice(QObject *parent);

    qint64 writeData(const char *buffer, qint64 size) override;
    qint64 readData(char *data, qint64 maxlen) override;

    void setCallback(const std::function<void (const char *, qint64)> &callback);

private:
    std::function<void(const char *buffer, qint64 size)> m_callback = nullptr;
};

#endif // INPUTDEVICE_H
