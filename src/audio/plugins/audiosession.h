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
#ifndef AUDIO_AUDIOSESSIONPLUGIN_H
#define AUDIO_AUDIOSESSIONPLUGIN_H

#include "../plugin.h"

namespace audio {

class AudioSessionPlugin : public Plugin
{
    Q_OBJECT
    friend void audioInterruptHandler(void *userData, unsigned int state);
    friend void backgroundModeHandler(void *userData, const bool &background);
    friend void routeChangedHandler(void *userData);
    const static DeviceInfo::Id OUTPUT_DEVICE_ID;
    const static DeviceInfo::Id INPUT_DEVICE_ID;

public:
    AudioSessionPlugin();
    ~AudioSessionPlugin();

    QString name() const override;
    DeviceInfo::List getDeviceInfoList() const override;
    DeviceInfo::Id defaultDeviceId(const Direction &mode) const override;

    Format deviceFormat(const DeviceInfo::Id &id, const Direction &mode) const override;
    Stream *open(const DeviceInfo::Id &, const Direction &mode, const Format &format, QIODevice *endpoint) override;

    bool inInterrupt() const;
    bool inBackground() const;

signals:
    void stopStreams(QPrivateSignal);
    void restoreStreams(QPrivateSignal);

private:
    void beginIterrupt();
    void endInterrupt();

    void beginBackground();
    void endBackground();

    void stopQueues();
    bool m_permission, m_inInterrupt, m_inBackground;
    mutable DeviceInfo::List m_deviceList;
};

} // namespace audio

#endif // AUDIO_AUDIOSESSIONPLUGIN_H
