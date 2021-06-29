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
#ifndef AUDIO_PLUGIN_H
#define AUDIO_PLUGIN_H

#include <map>
#include <QString>
#include <QObject>
#include <QIODevice>
#include "deviceinfo.h"
#include "stream.h"

namespace audio {

class Plugin : public QObject
{
    Q_OBJECT

public:
    Plugin();
    virtual ~Plugin();

    enum Direction {Input, Output};

    virtual QString name() const = 0;
    virtual DeviceInfo::List getDeviceInfoList() const = 0;
    virtual DeviceInfo::Id defaultDeviceId(const Direction &mode) const = 0;

    virtual Format deviceFormat(const DeviceInfo::Id &id, const Direction &mode) const = 0;
    virtual Stream *open(const DeviceInfo::Id &id, const Direction &mode, const Format &format, QIODevice *endpoint) = 0;

signals:
    void deviceListChanged();

protected:
    QString m_error = {};
};

} // namespace audio

#endif // AUDIO_PLUGIN_H
