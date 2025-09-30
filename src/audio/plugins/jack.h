/**
 *  OSM
 *  Copyright (C) 2022  Adrian Schollmeyer

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
#ifndef AUDIO_JACKPLUGIN_H
#define AUDIO_JACKPLUGIN_H

#include "../plugin.h"
#include <QObject>
#include <functional>
#include <jack/jack.h>
#include <jack/types.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

namespace audio {
class JackClient : public QObject
{
    Q_OBJECT

public:
    constexpr static std::size_t BUFFER_SIZE_MULTIPLIER{4};

    JackClient(const Plugin::Direction &direction, QIODevice& endpoint);
    JackClient(JackClient&&) = default;
    JackClient(const JackClient &) = delete;
    ~JackClient();

    void activate();
    void deactivate();

    jack_nframes_t currentSampleRate() const;
    std::size_t currentChannelCount() const;

    Plugin::Direction direction() const;

signals:
    void sampleRateChanged(jack_nframes_t newSampleRate);

public slots:
    void close();

private:
    void initJackClient();
    void initPorts();
    void initSampleRate();
    void initBufferSize();
    void initProcessing();

    void registerInputs();
    void registerOutputs();

    std::function<void(jack_client_t *)> jackClientDeleter();
    static std::function<void(jack_port_t *)> jackPortDeleter(jack_client_t *client);

    static int jackSampleRateCallback(jack_nframes_t newSampleRate, void* obj);
    int jackSampleRateCallbackInt(jack_nframes_t newSampleRate);

    static int jackBufferSizeCallback(jack_nframes_t newBufferSize, void* obj);
    int jackBufferSizeCallbackInt(jack_nframes_t newBufferSize);

    static int jackProcessCallback(jack_nframes_t nframes, void* obj);
    int jackProcessCallbackInt(jack_nframes_t nframes);

    void processInputPorts(jack_nframes_t nframes);
    void processOutputPorts(jack_nframes_t nframes);

    std::unique_ptr<jack_client_t, std::function<void(jack_client_t *)>> m_client;
    std::vector<std::unique_ptr<jack_port_t, std::function<void(jack_port_t *)>>> m_ports;
    std::vector<float*> m_portBuffers;
    jack_nframes_t m_sampleRate;
    jack_nframes_t m_bufferSize;
    const Plugin::Direction m_direction;
    QIODevice& m_endpoint;
    std::vector<float> m_sampleBuffer;
    bool m_active{false};
};

class JackPlugin : public Plugin
{
    Q_OBJECT

public:
    JackPlugin();
    JackPlugin(const JackPlugin &) = delete;
    virtual ~JackPlugin() = default;

    QString name() const override;
    DeviceInfo::List getDeviceInfoList() const override;
    DeviceInfo::Id defaultDeviceId(const Direction &mode) const override;

    Format deviceFormat(const DeviceInfo::Id &id, const Direction &mode) const override;
    Stream *open(const DeviceInfo::Id &id, const Direction &mode, const Format &format, QIODevice *endpoint) override;

private:
    DeviceInfo::List m_list{};
    DeviceInfo m_defaultDeviceInfoInput;
    DeviceInfo m_defaultDeviceInfoOutput;
    std::map<Stream *, std::unique_ptr<JackClient>> m_clients{};
};

class JackPluginException : public std::runtime_error
{
public:
    template<typename T>
    JackPluginException(const T& arg)
        : std::runtime_error(arg)
    {}
};
}

#endif // AUDIO_JACKPLUGIN_H
