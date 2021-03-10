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
#ifndef AUDIO_ALSAPLUGIN_H
#define AUDIO_ALSAPLUGIN_H

#include "../plugin.h"

namespace audio {

class AlsaPlugin : public Plugin
{
    Q_OBJECT

public:
    AlsaPlugin();

    QString name() const override;
    DeviceInfo::List getDeviceInfoList() const override;
    DeviceInfo::Id defaultDeviceId(const Direction &mode) const override;

    Format deviceFormat(const DeviceInfo::Id &id, const Direction &mode) const override;
    Stream *open(const DeviceInfo::Id &id, const Direction &mode, const Format &format, QIODevice *endpoint) override;

private:
    mutable DeviceInfo::List m_list;
    mutable QMap<Direction, DeviceInfo::Id> m_default;
};

} // namespace audio

#endif // AUDIO_ALSAPLUGIN_H
