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
#include <QCoreApplication>
#define ALSA_BUFFER_SIZE 1024
struct pcm_guard {
    explicit pcm_guard() {};

    int open(const char *name, snd_pcm_stream_t stream, int mode)
    {
        return snd_pcm_open(&handle, name, stream, mode);
    }

    void release()
    {
        handle = nullptr;
    }

    ~pcm_guard()
    {
        if (handle) {
            snd_pcm_close(handle);
        }
    }
    snd_pcm_t *handle = nullptr;
};

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

# define checkCall(foo, returnval) \
    if (!checkStatus(foo, QString::number(__LINE__))) {\
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
    char *cardName = nullptr;
    snd_pcm_t *pcm = nullptr;

    while (checkStatus(snd_card_next(&cardIndex), "") && (cardIndex > -1 )) {
        snd_card_get_name(cardIndex, &cardName);

        char plugId[127] = {0}, hwId[127] = {0};
        snprintf(hwId, sizeof(hwId), "hw:%d,0", cardIndex);
        snprintf(plugId, sizeof(plugId), "plughw:%d,0", cardIndex);
        DeviceInfo deviceInfo(plugId, name());
        deviceInfo.setName(cardName);

        if (snd_pcm_open(&pcm, plugId, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) == 0) {
            snd_pcm_hw_params_t *hw;
            snd_pcm_hw_params_alloca(&hw);
            snd_pcm_hw_params_any(pcm, hw);
            if (snd_pcm_hw_params_test_rate(pcm, hw, 48000, 0) == 0) {
                deviceInfo.setDefaultSampleRate(48000);
            } else {
                unsigned int sampleRate = 1;
                snd_pcm_hw_params_get_rate(hw, &sampleRate, 0);
                deviceInfo.setDefaultSampleRate(sampleRate);
            }
            snd_pcm_close(pcm);
        }

        if (snd_pcm_open(&pcm, hwId, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK) == 0) {
            snd_pcm_hw_params_t *hw;
            snd_pcm_hw_params_alloca(&hw);
            snd_pcm_hw_params_any(pcm, hw);
            unsigned int channels = 0;
            snd_pcm_hw_params_get_channels_max(hw, &channels);
            QStringList channelNames = {};
            channelNames.reserve(channels);
            for (unsigned int currentChannel = 1; currentChannel <= channels; ++currentChannel) {
                channelNames.push_back(QString::number(currentChannel));
            }
            deviceInfo.setInputChannels(channelNames);
            if (m_default[Direction::Input].isNull()) {
                m_default[Direction::Input] = QString(plugId);
            }
            snd_pcm_close(pcm);
        }

        if (snd_pcm_open(&pcm, hwId, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) == 0) {
            snd_pcm_hw_params_t *hw;
            snd_pcm_hw_params_alloca(&hw);
            snd_pcm_hw_params_any(pcm, hw);
            unsigned int channels = 0;
            snd_pcm_hw_params_get_channels_max(hw, &channels);
            QStringList channelNames = {};
            channelNames.reserve(channels);
            for (unsigned int currentChannel = 1; currentChannel <= channels; ++currentChannel) {
                channelNames.push_back(QString::number(currentChannel));
            }
            deviceInfo.setOutputChannels(channelNames);
            if (m_default[Direction::Output].isNull()) {
                m_default[Direction::Output] = QString(plugId);
            }
            snd_pcm_close(pcm);
        }
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
    if (id.isNull()) {
        return nullptr;
    }

    AlsaPCMDevice *device = m_devices[ {mode, id}];
    if (!device) {
        device = new AlsaPCMDevice(id, mode, format);
        connect (device, &QObject::destroyed, this, [this, mode, id]() {
            auto it = m_devices.find({mode, id});
            if (it != m_devices.end()) {
                m_devices.erase(it);
            }
        });

        m_devices[ {mode, id}] = device;
    }
    if (!device->start()) {
        return nullptr;
    }

    auto stream = new Stream(device->format());
    endpoint->open(mode == Input ? QIODevice::WriteOnly : QIODevice::ReadOnly);
    connect(stream, &Stream::closeMe, device, [endpoint, stream]() {
        if (endpoint->isOpen()) {
            endpoint->close();
        }
        usleep(30'000);//30ms
        stream->deleteLater();
    }, Qt::DirectConnection);
    AlsaPCMDevice::Callback endpointCallback;
    switch (mode) {
    case Input:
        endpointCallback = [endpoint](float * buffer, size_t size) {
            if (endpoint->isWritable()) {
                endpoint->write(reinterpret_cast<char *>(buffer), size * sizeof(float));
            }
        };
        break;
    case Output:
        endpointCallback = [endpoint](float * buffer, size_t size) {
            if (endpoint->isReadable()) {
                endpoint->read(reinterpret_cast<char *>(buffer), size * sizeof(float));
            }
        };
        break;
    }

    QMetaObject::invokeMethod(device, "addCallback", Qt::QueuedConnection,
                              Q_ARG(Stream *, stream),
                              Q_ARG(audio::AlsaPCMDevice::Callback, endpointCallback));

    return stream;
}

bool checkStatus(int errorCode, QString message, std::list<int> goodCodes)
{
    if (std::find(goodCodes.begin(), goodCodes.end(), errorCode) != goodCodes.end()) {
        return true;
    }
    qCritical() << "ALSA error: " << errorCode << snd_strerror(errorCode) << message;
    return false;
}

AlsaPCMDevice::AlsaPCMDevice(const DeviceInfo::Id &id, const Plugin::Direction &mode,
                             const Format &format) : QObject(), m_thread(nullptr), m_callbacks(),
    m_buffer(), m_id(id), m_mode(mode), m_format(format)
{
    m_buffer.resize(ALSA_BUFFER_SIZE * format.channelCount);
}

AlsaPCMDevice::~AlsaPCMDevice()
{
    if (m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait();
    }
    qDebug() << "~AlsaPCMDevice";
}

bool AlsaPCMDevice::start()
{
    if (m_handle) {
        return true;
    }
    auto streamFormat = m_format;
    unsigned int sr = m_format.sampleRate;
    pcm_guard pcm;
    snd_pcm_hw_params_t *hw;
    snd_pcm_stream_t direction = (m_mode == Plugin::Input ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK);
    snd_pcm_hw_params_alloca(&hw);
    checkCall(pcm.open(m_id.toLocal8Bit().data(), direction, 0), false);

    snd_guard<snd_pcm_hw_params_t, snd_pcm_hw_params_malloc, snd_pcm_hw_params_free> hwInfo;
    checkCall(snd_pcm_hw_params_any(pcm.handle, hw), false);

    checkCall(snd_pcm_hw_params_set_rate_resample(pcm.handle, hw, 1), false);
    checkCall(snd_pcm_hw_params_set_access(pcm.handle, hw, SND_PCM_ACCESS_RW_INTERLEAVED), false);

    checkCall(snd_pcm_hw_params_set_format(pcm.handle, hw, SND_PCM_FORMAT_FLOAT_LE), false);
    checkCall(snd_pcm_hw_params_set_channels(pcm.handle, hw, streamFormat.channelCount), false);
    checkCall(snd_pcm_hw_params_set_rate_near(pcm.handle, hw, &sr, 0), false);
    streamFormat.sampleRate = sr;
    checkCall(snd_pcm_hw_params_set_buffer_size(pcm.handle, hw, ALSA_BUFFER_SIZE), false);
    checkCall(snd_pcm_hw_params(pcm.handle, hw), false);

    m_thread = QThread::create(
    [handle = pcm.handle, samples = m_buffer.data(), this]() {
        while (true) {
            snd_pcm_sframes_t frameCount = 0;
            if (m_mode == Plugin::Input) {
                frameCount = snd_pcm_readi(handle, samples, ALSA_BUFFER_SIZE);
            }
            for (auto it = m_callbacks.begin(); it != m_callbacks.end();) {
                if (Q_UNLIKELY(!it.key()->active())) {
                    it = m_callbacks.erase(it);
                } else {
                    it.value()(m_buffer.data(), m_buffer.size());
                    ++it;
                }
            }
            if (m_mode == Plugin::Output) {
                frameCount = snd_pcm_writei(handle, samples, ALSA_BUFFER_SIZE);
            }
            if (frameCount < 0) {
                switch (-frameCount) {
                case EPIPE:
                    snd_pcm_prepare(handle);
                    break;
                case ESTRPIPE:
                    while (snd_pcm_resume(handle) == -EAGAIN) {
                        sleep(1);
                        if (snd_pcm_prepare(handle) < 0) {
                            qCritical("can't resume");
                            break;
                        }
                    }
                }
            }
            QCoreApplication::processEvents();
            if (m_callbacks.empty()) {
                break;
            }
        }
        qDebug() << "QThread finished";
    });


    if (!m_thread) {
        return false;
    }
    connect(m_thread, &QThread::finished, this, [this]() {
        stop();
        deleteLater();
    });
    moveToThread(m_thread);
    m_thread->start(QThread::TimeCriticalPriority);

    m_handle = pcm.handle;
    pcm.release();
    return true;
}

void AlsaPCMDevice::stop()
{
    if (m_handle) {
        snd_pcm_close(m_handle);
    }
}

bool AlsaPCMDevice::isOpen() const
{
    return m_handle != nullptr;
}

Format AlsaPCMDevice::format() const
{
    return m_format;
}

snd_pcm_t *AlsaPCMDevice::handle() const
{
    return m_handle;
}

void AlsaPCMDevice::addCallback(Stream *stream, Callback callback)
{
    m_callbacks[stream] = callback;
}

} // namespace audio
