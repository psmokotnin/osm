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
#include "jack.h"
#include <algorithm>
#include <cassert>

namespace {
constexpr unsigned int DEFAULT_SAMPLE_RATE{48000};
constexpr unsigned int DEFAULT_CHANNEL_COUNT_INPUT{2};
constexpr unsigned int DEFAULT_CHANNEL_COUNT_OUTPUT{1};

constexpr const char* PLUGIN_NAME{"JACK"};
constexpr const char* JACK_CLIENT_NAME{"OpenSoundMeter"};
constexpr const char* DEFAULT_MEASUREMENT_CHANNEL{"measurement"};
constexpr const char* DEFAULT_REFERENCE_CHANNEL{"reference"};
constexpr const char* DEFAULT_OUTPUT_CHANNEL{"out"};
}

namespace audio {
JackClient::JackClient(const Plugin::Direction &direction, QIODevice& endpoint)
    : m_client(nullptr, jackClientDeleter()), m_direction(direction), m_endpoint(endpoint)
{
    initJackClient();
    initPorts();
    initSampleRate();
    initBufferSize();
    initProcessing();
}

JackClient::~JackClient()
{
    // Explicitly stop processing first, to prevent us still running the JACK
    // processor while the rest of the object is being cleaned up
    if (m_client)
        close();
}

jack_nframes_t JackClient::currentSampleRate() const
{
    return m_sampleRate;
}

std::size_t JackClient::currentChannelCount() const
{
    return m_ports.size();
}

Plugin::Direction JackClient::direction() const
{
    return m_direction;
}

void JackClient::close()
{
    deactivate();
    m_client.reset(nullptr);
}

void JackClient::initJackClient()
{
    jack_status_t jackStatus;
    jack_client_t* jackClient = jack_client_open(
        JACK_CLIENT_NAME,
        JackOptions::JackNoStartServer,
        &jackStatus
    );

    if (!jackClient)
        throw JackPluginException{"Failed to open JACK client"};

    m_client.reset(jackClient);
}

void JackClient::initSampleRate()
{
    assert(m_client);
    m_sampleRate = jack_get_sample_rate(m_client.get());
    qDebug() << "[JACK] Sample rate set: " << m_sampleRate;

    int res = jack_set_sample_rate_callback(
        m_client.get(),
        jackSampleRateCallback,
        this
    );
    if (res != 0)
        throw JackPluginException{"Failed to register sample rate callback"};
}

void JackClient::initBufferSize()
{
    assert(m_client);
    assert(!m_ports.empty());
    m_bufferSize = jack_get_buffer_size(m_client.get());
    m_sampleBuffer.resize(m_bufferSize * m_ports.size());
    qDebug() << "[JACK] Buffer size set: " << m_bufferSize;

    int res = jack_set_buffer_size_callback(
        m_client.get(),
        jackBufferSizeCallback,
        this
    );
    if (res != 0)
        throw JackPluginException{"Failed to register buffer size callback"};
}

void JackClient::initPorts()
{
    assert(m_client);
    switch (m_direction) {
    case Plugin::Direction::Input:
        registerInputs();
        break;

    case Plugin::Direction::Output:
        registerOutputs();
        break;

    default:
        throw JackPluginException{"Unsupported signal direction"};
    }
}

void JackClient::initProcessing()
{
    assert(m_client);
    int res = jack_set_process_callback(
        m_client.get(),
        jackProcessCallback,
        this
    );
    if (res != 0)
        throw JackPluginException{"Failed to register process callback"};
}

void JackClient::activate()
{
    assert(m_client);
    if (m_active)
        return;

    qDebug() << "[JACK] Activating";
    int res = jack_activate(m_client.get());
    if (res != 0)
        throw JackPluginException{"Failed to activate client"};

    m_active = true;
}

void JackClient::deactivate()
{
    if (!m_active)
        return;

    m_active = false;

    assert(m_client);
    int res = jack_deactivate(m_client.get());
    if (res != 0)
        throw JackPluginException{"Failed to deactivate client"};
}

void JackClient::registerInputs()
{
    qDebug() << "[JACK] Registering Inputs";
    jack_port_t *measurementPort = jack_port_register(
        m_client.get(),
        DEFAULT_MEASUREMENT_CHANNEL,
        JACK_DEFAULT_AUDIO_TYPE,
        JackPortFlags::JackPortIsInput,
        0
    );
    if (!measurementPort)
        throw JackPluginException{"Failed to register measurement port"};
    m_ports.emplace_back(measurementPort, jackPortDeleter(m_client.get()));
    m_portBuffers.push_back(nullptr);

    jack_port_t *referencePort = jack_port_register(
        m_client.get(),
        DEFAULT_REFERENCE_CHANNEL,
        JACK_DEFAULT_AUDIO_TYPE,
        JackPortFlags::JackPortIsInput,
        0
    );
    if (!referencePort)
        throw JackPluginException{"Failed to register reference port"};
    m_ports.emplace_back(referencePort, jackPortDeleter(m_client.get()));
    m_portBuffers.push_back(nullptr);
}

void JackClient::registerOutputs()
{
    qDebug() << "[JACK] Registering Outputs";
    jack_port_t *outputPort = jack_port_register(
        m_client.get(),
        DEFAULT_OUTPUT_CHANNEL,
        JACK_DEFAULT_AUDIO_TYPE,
        JackPortFlags::JackPortIsOutput,
        0
    );
    if (!outputPort)
        throw JackPluginException{"Failed to register output port"};
    m_ports.emplace_back(outputPort, jackPortDeleter(m_client.get()));
    m_portBuffers.push_back(nullptr);
}

std::function<void(jack_client_t *)> JackClient::jackClientDeleter()
{
    return [this](jack_client_t *client) -> void {
        m_ports.resize(0);
        jack_client_close(client);
    };
}

std::function<void(jack_port_t *)> JackClient::jackPortDeleter(jack_client_t *client)
{
    // We can't use the object's client member here as it might already have
    // changed if this deleter is called in the client object's deleter,
    // resulting in an assertion failure or segfault.
    assert(client);
    return [client](jack_port_t *port) -> void {
        jack_port_unregister(client, port);
    };
}

int JackClient::jackSampleRateCallback(jack_nframes_t newSampleRate, void* obj)
{
    assert(obj);
    return reinterpret_cast<JackClient*>(obj)->jackSampleRateCallbackInt(newSampleRate);
}

int JackClient::jackSampleRateCallbackInt(jack_nframes_t newSampleRate)
{
    m_sampleRate = newSampleRate;
    qInfo() << "[JACK] Sample rate updated: " << m_sampleRate;
    emit sampleRateChanged(m_sampleRate);
    return 0;
}

int JackClient::jackBufferSizeCallback(jack_nframes_t newBufferSize, void* obj)
{
    assert(obj);
    return reinterpret_cast<JackClient*>(obj)->jackBufferSizeCallbackInt(newBufferSize);
}

int JackClient::jackBufferSizeCallbackInt(jack_nframes_t newBufferSize)
{
    m_bufferSize = newBufferSize;
    qInfo() << "[JACK] Buffer size updated: " << m_bufferSize;
    if (newBufferSize > 0)
        m_sampleBuffer.resize(newBufferSize * m_ports.size());

    return 0;
}

int JackClient::jackProcessCallback(jack_nframes_t nframes, void* obj)
{
    assert(obj);
    return reinterpret_cast<JackClient*>(obj)->jackProcessCallbackInt(nframes);
}

int JackClient::jackProcessCallbackInt(jack_nframes_t nframes)
{
    // Yes, this can actually happen
    if (m_bufferSize == 0)
        return 1;

    if (!m_active)
        return 0;

    assert(m_ports.size() == m_portBuffers.size());
    assert(nframes == m_bufferSize);
    assert(m_endpoint.isOpen());

    if (m_direction == Plugin::Direction::Input) {
        processInputPorts(nframes);
    } else {
        processOutputPorts(nframes);
    }
    return 0;
}

void JackClient::processInputPorts(jack_nframes_t nframes)
{
    if (!m_endpoint.isWritable())
        return;

    for (std::size_t i = 0; i < m_ports.size(); ++i) {
        m_portBuffers[i] = reinterpret_cast<float*>(
            jack_port_get_buffer(m_ports[i].get(), nframes));
    }

    std::size_t sampleBufferPos{0};
    for (jack_nframes_t i = 0; i < nframes; ++i) {
        for (const float* portBuf : m_portBuffers) {
            assert(sampleBufferPos < m_sampleBuffer.size());
            m_sampleBuffer[sampleBufferPos] = portBuf[i];
            sampleBufferPos++;
        }
    }
    assert(sampleBufferPos == nframes * m_portBuffers.size());
    m_endpoint.write(
        reinterpret_cast<char*>(m_sampleBuffer.data()),
        sampleBufferPos * sizeof(float)
    );
}

void JackClient::processOutputPorts(jack_nframes_t nframes)
{
    if (!m_endpoint.isReadable())
        return;

    for (std::size_t i = 0; i < m_ports.size(); ++i) {
        m_portBuffers[i] = reinterpret_cast<float*>(
            jack_port_get_buffer(m_ports[i].get(), nframes));
    }

    const std::size_t samplesToWrite = std::max(
        static_cast<std::size_t>(nframes),
        static_cast<std::size_t>(m_endpoint.bytesAvailable() / sizeof(float))
    );
    assert(samplesToWrite % m_ports.size() == 0);

    std::size_t portIdx{0};
    std::size_t portBufferPos{0};
    for (std::size_t i = 0; i < samplesToWrite; ++i) {
        assert(portIdx < m_portBuffers.size());
        assert(portBufferPos < m_bufferSize);
        const std::size_t bytesRead = m_endpoint.read(
            reinterpret_cast<char*>(m_portBuffers[portIdx] + portBufferPos),
            sizeof(float)
        );
        assert(bytesRead == sizeof(float));

        portIdx = (portIdx + 1) % m_portBuffers.size();
        if (portIdx == 0)
            portBufferPos++;
    }
}

JackPlugin::JackPlugin()
{
    DeviceInfo m_defaultDeviceInfo{PLUGIN_NAME, PLUGIN_NAME};
    m_defaultDeviceInfo.setName(PLUGIN_NAME);
    m_defaultDeviceInfo.setInputChannels({
        DEFAULT_MEASUREMENT_CHANNEL,
        DEFAULT_REFERENCE_CHANNEL
    });
    m_defaultDeviceInfo.setOutputChannels({DEFAULT_OUTPUT_CHANNEL});
    m_defaultDeviceInfo.setDefaultSampleRate(DEFAULT_SAMPLE_RATE);
    m_list.push_back(m_defaultDeviceInfo);
}

QString JackPlugin::name() const
{
    return PLUGIN_NAME;
}

DeviceInfo::List JackPlugin::getDeviceInfoList() const
{
    return m_list;
}

DeviceInfo::Id JackPlugin::defaultDeviceId(
    [[maybe_unused]] const Plugin::Direction &mode) const
{
    return DeviceInfo::Id();
}

Format JackPlugin::deviceFormat(
    [[maybe_unused]] const DeviceInfo::Id &id,
    const Plugin::Direction &mode) const
{
    const auto client_it = std::find_if(
        m_clients.begin(),
        m_clients.end(),
        [mode](const auto& client) {

        return client.second->direction() == mode;
    });

    if (client_it == m_clients.end()) {
        if (mode == Plugin::Direction::Input)
            return {DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_COUNT_INPUT};
        else
            return {DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_COUNT_OUTPUT};
    } else {
        return {
            static_cast<unsigned int>(client_it->second->currentSampleRate()),
            static_cast<unsigned int>(client_it->second->currentChannelCount())
        };
    }
}

Stream *JackPlugin::open(
    [[maybe_unused]] const DeviceInfo::Id &id,
    const Plugin::Direction &mode,
    [[maybe_unused]] const Format &format,
    QIODevice *endpoint)
{
    // The sample rate is set by JACK, so we don't mess with it.
    // The channel count is fixed to the minimum amount of channels necessary
    // for OSM to work as we can freely patch the cannels in JACK.
    //
    // The ID can be ignored as we create individual JACK clients
    // for each requested Stream.
    try {
        std::unique_ptr<JackClient> jack = std::make_unique<JackClient>(mode, *endpoint);

        Format f{
            jack->currentSampleRate(),
            static_cast<unsigned int>(jack->currentChannelCount())
        };
        Stream *stream = new Stream(f);

        m_clients.emplace(stream, std::move(jack));
        JackClient& jackClient = *m_clients.at(stream).get();

        connect(stream, &Stream::closeMe, this, [this, stream, endpoint]() {
            // Order is important here to prevent JACK still writing data to
            // the endpoint after it has been closed.
            m_clients.erase(stream);

            if (endpoint->isOpen())
                endpoint->close();

            stream->deleteLater();
        });

        connect(
            &jackClient,
            &JackClient::sampleRateChanged,
            this,
            [stream](jack_nframes_t newRate) {
                assert(stream);
                stream->setSampleRate(static_cast<unsigned int>(newRate));
        });

        endpoint->open(mode == Input ? QIODevice::WriteOnly : QIODevice::ReadOnly);
        jackClient.activate();

        return stream;
    } catch (JackPluginException& e) {
        return nullptr;
    }
}

}
