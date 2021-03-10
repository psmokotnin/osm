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
#include "alsa.h"
#include <cstring>
#include <alsa/asoundlib.h>

template <typename T, int(*Alloc)(T **), void(*Free)(T *)> struct snd_guard {
    explicit snd_guard()
    {
        auto errorCode = Alloc(&data);
        if (errorCode != 0) {
            data = nullptr;
            qCritical() << "Alloc failed" << snd_strerror(errorCode);
        }
    }
    ~snd_guard()
    {
        if (data) {
            Free(data);
        }
    }
    T *operator->()
    {
        return data;
    }

    T *data = nullptr;
};

namespace audio {

bool checkStatus(int errorCode, QString message = "", std::list<int> goodStatuses = {0});

# define checkCall(foo, returnval, message) \
    if (!checkStatus(foo, message)) {\
    return returnval;\
};

AlsaPlugin::AlsaPlugin() : m_list()
{
    m_default[Direction::Input] = DeviceInfo::Id();
    m_default[Direction::Output] = DeviceInfo::Id();
}

QString AlsaPlugin::name() const
{
    return "ALSA";
}

DeviceInfo::List AlsaPlugin::getDeviceInfoList() const
{
    DeviceInfo::List list = {};
    int cardIndex = -1;
    char *deviceName = nullptr;
    snd_ctl_t *ctl = nullptr;
    snd_pcm_t *pcm = nullptr;
    while (checkStatus(snd_card_next(&cardIndex), "") && (cardIndex > -1 )) {
        snd_card_get_name(cardIndex, &deviceName);
        snd_guard<snd_pcm_hw_params_t, snd_pcm_hw_params_malloc, snd_pcm_hw_params_free> hwInfo;

        char alsaName[127] = {0};
        snprintf(alsaName, sizeof(alsaName), "hw:%d", cardIndex);
        if (!checkStatus(snd_ctl_open(&ctl, alsaName, 0), "snd_ctl_open")) {
            continue;
        }
        DeviceInfo deviceInfo(alsaName, name());
        deviceInfo.setName(deviceName);

        auto checkSR = [&deviceInfo](auto pcm, auto hw) {
            if (deviceInfo.defaultSampleRate()) {
                return;
            }
            if (snd_pcm_hw_params_test_rate(pcm, hw, 48000, 0) == 0) {
                deviceInfo.setDefaultSampleRate(48000);
                return;
            }
            unsigned int rate = 1, denominator = 1;
            if (snd_pcm_hw_params_get_rate_numden(hw, &rate, &denominator) == 0) {
                deviceInfo.setDefaultSampleRate(rate / denominator);
            }
        };

        if (snd_pcm_open(&pcm, alsaName, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK) == 0) {
            snd_pcm_hw_params_any(pcm, hwInfo.data);
            unsigned int channels = 0;
            snd_pcm_hw_params_get_channels_max(hwInfo.data, &channels);
            QStringList channelNames = {};
            channelNames.reserve(channels);
            for (unsigned int currentChannel = 1; currentChannel <= channels; ++currentChannel) {
                channelNames.push_back(QString::number(currentChannel));
            }
            deviceInfo.setInputChannels(channelNames);
            checkSR(pcm, hwInfo.data);
            if (m_default[Direction::Input].isNull()) {
                m_default[Direction::Input] = QString(alsaName);
            }
            snd_pcm_close(pcm);
        }

        if (snd_pcm_open(&pcm, alsaName, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) == 0) {
            snd_pcm_hw_params_any(pcm, hwInfo.data);
            unsigned int channels = 0;
            snd_pcm_hw_params_get_channels_max(hwInfo.data, &channels);
            QStringList channelNames = {};
            channelNames.reserve(channels);
            for (unsigned int currentChannel = 1; currentChannel <= channels; ++currentChannel) {
                channelNames.push_back(QString::number(currentChannel));
            }
            deviceInfo.setOutputChannels(channelNames);
            checkSR(pcm, hwInfo.data);
            if (m_default[Direction::Output].isNull()) {
                m_default[Direction::Output] = QString(alsaName);
            }
            snd_pcm_close(pcm);
        }
        snd_ctl_close(ctl);
        ctl = nullptr;

        list << deviceInfo;
    }
    m_list = list;
    return m_list;
}

DeviceInfo::Id AlsaPlugin::defaultDeviceId(const Plugin::Direction &mode) const
{
    return m_default[mode];
}

Format AlsaPlugin::deviceFormat(const DeviceInfo::Id &id, const Plugin::Direction &mode) const
{
    auto deviceIt = std::find_if(m_list.cbegin(), m_list.cend(), [&id](auto d) {
        return d.id() == id;
    });
    if (deviceIt == m_list.cend()) {
        return {1, 0};
    }
    auto device = *deviceIt;
    unsigned int count = (mode == Input ? device.inputChannels().count() : device.outputChannels().count());
    return {
        device.defaultSampleRate(),
        count
    };
}

Stream *AlsaPlugin::open(const DeviceInfo::Id &id, const Plugin::Direction &mode, const Format &format,
                         QIODevice *endpoint)
{
    snd_pcm_t *handle;
    snd_pcm_stream_t direction = (mode == Input ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK);
    checkCall(snd_pcm_open(&handle, id.toLocal8Bit().data(), direction, SND_PCM_NONBLOCK /*& SND_PCM_ASYNC*/), nullptr,
              "pcm stream open");

    snd_guard<snd_pcm_hw_params_t, snd_pcm_hw_params_malloc, snd_pcm_hw_params_free> hwInfo;
    checkCall(snd_pcm_hw_params_any(handle, hwInfo.data), nullptr, "snd_pcm_hw_params_any");

    checkCall(snd_pcm_hw_params_set_rate_resample(handle, hwInfo.data, 1), nullptr, QString::number(__LINE__));
    checkCall(snd_pcm_hw_params_set_access(handle, hwInfo.data, SND_PCM_ACCESS_RW_INTERLEAVED), nullptr,
              QString::number(__LINE__));
    snd_pcm_format_t f;
    unsigned int sr;
    snd_pcm_hw_params_get_format(hwInfo.data, &f);
    snd_pcm_hw_params_get_rate(hwInfo.data, &sr, 0);
    qDebug() << f << SND_PCM_FORMAT_S16_LE << sr;
    checkCall(snd_pcm_hw_params_set_format(handle, hwInfo.data, SND_PCM_FORMAT_FLOAT_LE), nullptr,
              QString::number(__LINE__));
    checkCall(snd_pcm_hw_params_set_channels(handle, hwInfo.data, format.channelCount), nullptr, QString::number(__LINE__));
    sr = format.sampleRate;
    checkCall(snd_pcm_hw_params_set_rate_near(handle, hwInfo.data, &sr, 0), nullptr, QString::number(__LINE__));
    checkCall(snd_pcm_hw_params(handle, hwInfo.data), nullptr, QString::number(__LINE__));


    //on finish
    if (0) {
        snd_pcm_close(handle);
    }
    return nullptr;
}


bool checkStatus(int errorCode, QString message, std::list<int> goodCodes)
{
    if (std::find(goodCodes.begin(), goodCodes.end(), errorCode) != goodCodes.end()) {
        return true;
    }
    qCritical() << "ALSA error: " << errorCode << snd_strerror(errorCode) << message;
    return false;
}
} // namespace audio
