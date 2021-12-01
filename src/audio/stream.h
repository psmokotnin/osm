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
#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <atomic>
#include <QObject>
#include "format.h"

namespace audio {

class Stream : public QObject
{
    Q_OBJECT

public:
    Stream(const Format &format);

    void close();
    Format format() const;
    bool active() const;
    void setSampleRate(unsigned int sampleRate);

    size_t depth() const;
    void setDepth(const size_t &depth);

signals:
    void closeMe();
    void sampleRateChanged();

private:
    Format m_format;
    std::atomic<bool> m_active;
    size_t m_depth;
};

} // namespace audio

#endif // AUDIO_STREAM_H
