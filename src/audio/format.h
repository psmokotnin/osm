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
#ifndef AUDIO_FORMAT_H
#define AUDIO_FORMAT_H

#include <QDebug>

namespace audio {

struct Format {
    Format() = default;
    Format(const Format &other) = default;
    bool isValid() const;

    unsigned int sampleRate = 0;
    unsigned int channelCount = 0;
};

} // namespace audio

QDebug operator<<(QDebug dbg, const audio::Format &f);

#endif // AUDIO_FORMAT_H
