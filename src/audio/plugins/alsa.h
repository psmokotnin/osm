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

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#undef ALSA_PCM_NEW_HW_PARAMS_API
#undef ALSA_PCM_NEW_SW_PARAMS_API

#include "../plugin.h"
#include <QThread>
namespace audio {
class AlsaPCMDevice;
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
    QHash<std::pair<Direction, DeviceInfo::Id>, AlsaPCMDevice *> m_devices;
    std::mutex m_deviceListMutex;
};

class AlsaPCMDevice : public QObject
{
    Q_OBJECT

public:
    using Callback = std::function<void(float *, size_t)>;
    AlsaPCMDevice(const DeviceInfo::Id &id, const Plugin::Direction &mode, const Format &format, std::mutex &mutex);
    ~AlsaPCMDevice();

    bool start();
    void stop();

    Format format() const;
    bool active() const;

    void setKeepAlive();

public slots:
    void addCallback(Stream *stream, audio::AlsaPCMDevice::Callback callback);
    void removeCallback(Stream *stream);

signals:
    void closed();

private:
    QThread *m_thread;
    QHash<Stream *, Callback> m_callbacks;
    std::vector<float> m_buffer;
    DeviceInfo::Id m_id;
    Plugin::Direction m_mode;
    Format m_format;
    snd_pcm_t *m_handle = nullptr;
    std::mutex &m_mutex;
    std::atomic<bool> m_keepAlive, m_threadActive;
};
} // namespace audio

Q_DECLARE_METATYPE(audio::AlsaPCMDevice::Callback)
#endif // AUDIO_ALSAPLUGIN_H
