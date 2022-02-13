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

#include <initguid.h>
#include "wasapi.h"
#include <comdef.h>
#include <Audioclient.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys.h>
#include <QThread>
#include <QCoreApplication>

namespace audio {

bool checkStatus(HRESULT result, QString message = "", std::list<HRESULT> goodStatuses = {S_OK});
# define checkCall(foo, returnval, message) \
    if (!checkStatus(foo, QString(message) + " " + QString::number(__LINE__))) {\
    return returnval;\
};

#define checkEnumerator(returnval) \
    if (!m_enumerator) { \
        qCritical() << "no enumerator"; \
        return returnval; \
    };

WasapiPlugin::WasapiPlugin()
{
    checkStatus(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                 CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                 reinterpret_cast<LPVOID *>(&m_enumerator)),
                "CoCreateInstance");
}

WasapiPlugin::~WasapiPlugin()
{
    if (m_enumerator) {
        m_enumerator->Release();
    }
}

QString WasapiPlugin::name() const
{
    return "WASAPI";
}

DeviceInfo::List WasapiPlugin::getDeviceInfoList() const
{
    checkEnumerator({});
    UINT count = 0;
    DeviceInfo::List list = {};
    IMMDeviceCollection *devices = nullptr;

    checkCall(m_enumerator->EnumAudioEndpoints(
                  EDataFlow::eAll,
                  DEVICE_STATE_ACTIVE,
                  &devices),
              list,
              "EnumAudioEndpoints");

    checkCall(devices->GetCount(&count), {}, "GetCount");

    for (ULONG i = 0; i < count; ++i) {
        IMMDevice *device = nullptr;
        IMMEndpoint *endpoint = nullptr;
        EDataFlow deviceFlow = eAll;
        IPropertyStore *properties = nullptr;
        WAVEFORMATEX *deviceFormat;
        UINT channelCount = 0;
        LPWSTR deviceID = nullptr;
        PROPVARIANT propertyValue;
        PropVariantInit(&propertyValue);

        checkCall(devices->Item(i, &device), list, "GetDevice");
        checkCall(device->GetId(&deviceID), list, "GetId");

        checkCall(device->QueryInterface(__uuidof(IMMEndpoint), reinterpret_cast<void **>(&endpoint)), list, "IMMEndpoint");
        checkCall(endpoint->GetDataFlow(&deviceFlow), list, "GetDataFlow");

        DeviceInfo info(QString::fromWCharArray(deviceID), name());

        if (list.indexOf(info) != -1) {
            continue;
        }
        checkCall(device->OpenPropertyStore(STGM_READ, &properties), list, "OpenPropertyStore");

        // PKEY_Device_FriendlyName
        static PROPERTYKEY key_Device_FriendlyName;
        GUID IDevice_FriendlyName = { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } };
        key_Device_FriendlyName.pid = 14;
        key_Device_FriendlyName.fmtid = IDevice_FriendlyName;

        checkCall(properties->GetValue(key_Device_FriendlyName, &propertyValue), list, "PKEY_Device_FriendlyName");
        info.setName(QString::fromWCharArray(propertyValue.pwszVal));

        checkCall(properties->GetValue(PKEY_AudioEngine_DeviceFormat, &propertyValue), list, "PKEY_AudioEngine_DeviceFormat");
        deviceFormat = reinterpret_cast<WAVEFORMATEX *>(propertyValue.blob.pBlobData);
        if (deviceFormat) {
            info.setDefaultSampleRate(deviceFormat->nSamplesPerSec);
            channelCount = deviceFormat->nChannels;
        }

        //TODO: FIX it! More than 2 outputs don't work in system
        if (deviceFlow == EDataFlow::eRender && channelCount > 2) {
            channelCount = 2;
        }

        QStringList channelNames = {};
        channelNames.reserve(channelCount);
        for (UINT currentChannel = 1; currentChannel <= channelCount; ++currentChannel) {
            channelNames.push_back(QString::number(currentChannel));
        }
        switch (deviceFlow) {
        case EDataFlow::eRender:
            info.setOutputChannels(channelNames);
            break;
        case EDataFlow::eCapture:
            info.setInputChannels(channelNames);
            break;
        default:
            ;
        }
        list.push_back(info);

        CoTaskMemFree(deviceFormat);
        properties->Release();
        device->Release();
    }
    devices->Release();
    return list;
}

DeviceInfo::Id WasapiPlugin::defaultDeviceId(const Plugin::Direction &mode) const
{
    checkEnumerator({});
    DeviceInfo::Id id = {};
    IMMDevice *device = nullptr;
    LPWSTR deviceID = nullptr;
    EDataFlow flow = (mode == Direction::Input ? eCapture : eRender);

    checkCall(m_enumerator->GetDefaultAudioEndpoint(flow, ERole::eMultimedia, &device),
              id,
              "GetDefaultAudioEndpoint");

    checkCall(device->GetId(&deviceID), id, "GetId");
    id = QString::fromWCharArray(deviceID);
    device->Release();

    return id;
}

Format WasapiPlugin::deviceFormat(const DeviceInfo::Id &id, const Plugin::Direction &mode) const
{
    checkEnumerator({});
    auto list = getDeviceInfoList();
    auto it = std::find_if(list.begin(), list.end(), [&id](auto & e) {
        return e.id() == id;
    });

    if (it == list.end()) {
        return {};
    }
    auto device = *it;
    unsigned int channelCount = (mode == Direction::Input ? device.inputChannels().count() :
                                 device.outputChannels().count());

    return {
        device.defaultSampleRate(),
        channelCount
    };
}

QDebug operator << (QDebug dbg, const WAVEFORMATEX &format)
{
    dbg.nospace() << "\nWAVEFORMATEX: {"
                    << "nChannels: "        << format.nChannels << " "
                    << "nAvgBytesPerSec: "  << format.nAvgBytesPerSec << " "
                    << "nBlockAlign: "      << format.nBlockAlign << " "
                    << "nSamplesPerSec: "   << format.nSamplesPerSec << " "
                    << "wBitsPerSample: "   << format.wBitsPerSample << " "
                    << "wFormatTag: "       << format.wFormatTag << " "
                    << "cbSize: "           << format.cbSize << " "
                    << "} \n";

        return dbg.maybeSpace();
}

QDebug operator << (QDebug dbg, const _GUID &guid)
{
    dbg.nospace() << "{"
                    << guid.Data1 << ":"
                    << guid.Data2 << ":"
                    << guid.Data3 << ":"
                    << guid.Data4[0] << guid.Data4[1] << guid.Data4[2] << guid.Data4[3]
                    << guid.Data4[4] << guid.Data4[5] << guid.Data4[6] << guid.Data4[7]
                    << "}";

        return dbg.maybeSpace();
}
Stream *WasapiPlugin::open(const DeviceInfo::Id &id, const Plugin::Direction &mode, const Format &format,
                           QIODevice *endpoint)
{
    checkEnumerator({});
    Stream *stream = nullptr;
    IMMDevice *device = nullptr;
    IAudioClient *client;
    UINT32 bufferSizeInFrames, bytesPerFrame;
    HANDLE streamReadyEvent = CreateEvent(nullptr, false, false, nullptr);
    LPWSTR deviceID = new WCHAR[id.length() * 2];
    auto end = id.toWCharArray(deviceID);
    deviceID[end] = '\0';
    if (!streamReadyEvent) {
        qCritical() << "can't start stream without event handle";
        return nullptr;
    }

    CoInitialize(nullptr);

    checkCall(m_enumerator->GetDevice(deviceID, &device), nullptr, "GetDevice");
    checkCall(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, reinterpret_cast<void **>(&client)),
              nullptr,
              "Activate IAudioClient");

    WAVEFORMATEX *p_waveFormat = nullptr;
    Format streamFormat = format;
    checkCall(client->GetMixFormat(&p_waveFormat), nullptr, "GitMixFormat");

    streamFormat.sampleRate = p_waveFormat->nSamplesPerSec;
    if (p_waveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
        p_waveFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        p_waveFormat->nChannels = streamFormat.channelCount;
        p_waveFormat->nSamplesPerSec = streamFormat.sampleRate;
        p_waveFormat->wBitsPerSample = sizeof(float) * 8;
        p_waveFormat->nAvgBytesPerSec = p_waveFormat->nChannels * p_waveFormat->nSamplesPerSec * p_waveFormat->wBitsPerSample / 8;
        p_waveFormat->nBlockAlign = p_waveFormat->nChannels * sizeof(float);
        p_waveFormat->cbSize = 0;
    }
    if (p_waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE){
        auto formatEx = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(p_waveFormat);
        formatEx->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        formatEx->Format.nChannels = streamFormat.channelCount;
        formatEx->dwChannelMask = std::pow(2, streamFormat.channelCount) - 1;

        //TODO: use to debug max 2 output channels
        //qDebug() << formatEx->Format.nChannels << Qt::bin << formatEx->dwChannelMask;
        formatEx->Format.nSamplesPerSec = streamFormat.sampleRate;
        formatEx->Format.wBitsPerSample = sizeof(float) * 8;
        formatEx->Format.nAvgBytesPerSec = formatEx->Format.nChannels * formatEx->Format.nSamplesPerSec * formatEx->Format.wBitsPerSample / 8;
        formatEx->Format.nBlockAlign = formatEx->Format.nChannels * sizeof(float);
        //formatEx->Format.cbSize = 22;

        formatEx->Samples.wValidBitsPerSample = 32;
        formatEx->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    }

    REFERENCE_TIME defaultPeriod;
    checkCall(client->GetDevicePeriod(&defaultPeriod, nullptr), nullptr, "GetDevicePeriod");

    checkCall(client->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                 AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
                                 defaultPeriod * 10, 0, p_waveFormat, NULL),
              nullptr,
              "Initialize audio client");

    checkCall(client->SetEventHandle(streamReadyEvent), nullptr, "SetEventHandle");

    checkCall(client->GetBufferSize(&bufferSizeInFrames), nullptr, "GetBufferSize");
    bytesPerFrame = p_waveFormat->nBlockAlign;
    CoTaskMemFree(p_waveFormat);

    switch (mode) {
    case Output: {
        IAudioRenderClient *renderClient = nullptr;
        checkCall(client->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void **>(&renderClient)),
                  nullptr,
                  "GetService IAudioRenderClient");

        checkCall(client->Start(), nullptr, "start client");

        endpoint->open(QIODevice::ReadOnly);

        stream = new Stream(streamFormat);
        QThread *audioEndpointThread = QThread::create(
        [renderClient, endpoint, streamReadyEvent, client, bufferSizeInFrames, bytesPerFrame]() {
            BYTE *data;
            UINT32 padding, framesAvailable;
            while (endpoint->isOpen() && (WaitForSingleObject(streamReadyEvent, 1000) == WAIT_OBJECT_0)) {
                checkStatus(client->GetCurrentPadding(&padding), "GetCurrentPadding");
                framesAvailable = bufferSizeInFrames - padding;
                if (checkStatus(renderClient->GetBuffer(framesAvailable, &data), "GetBuffer")) {
                    endpoint->read(reinterpret_cast<char *>(data), framesAvailable * bytesPerFrame);
                    checkStatus(renderClient->ReleaseBuffer(framesAvailable, 0x00), "ReleaseBuffer");
                }
            }
            checkStatus(client->Stop(), "stop client");
        });
        audioEndpointThread->start();

        connect(stream, &Stream::closeMe, this, [stream, endpoint, client, audioEndpointThread, renderClient,
                streamReadyEvent]() {
            endpoint->close();
            CloseHandle(streamReadyEvent);

            audioEndpointThread->quit();
            audioEndpointThread->wait();
            renderClient->Release();
            client->Release();
            stream->deleteLater();
        }, Qt::DirectConnection);
        break;
    }

    case Input: {
        IAudioCaptureClient *captureClient = nullptr;
        checkCall(client->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void **>(&captureClient)),
                  nullptr,
                  "GetService IAudioCaptureClient");

        checkCall(client->Start(), nullptr, "start client");

        endpoint->open(QIODevice::WriteOnly);
        stream = new Stream(streamFormat);

        QThread *audioEndpointThread = QThread::create(
        [stream, captureClient, endpoint, streamReadyEvent, client, bytesPerFrame]() {
            BYTE *data;
            UINT32 availableFramesCount;
            DWORD flags;

            while (endpoint->isOpen() && (WaitForSingleObject(streamReadyEvent, 1000) == WAIT_OBJECT_0)) {
                checkStatus(captureClient->GetNextPacketSize(&availableFramesCount), "GetNextPacketSize");
                if (availableFramesCount == 0) {
                    continue;
                }
                if (checkStatus(captureClient->GetBuffer(&data, &availableFramesCount, &flags, NULL, NULL), "GetBuffer")) {
                    if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                        endpoint->write(reinterpret_cast<char *>(data), availableFramesCount * bytesPerFrame);
                    }
                    checkStatus(captureClient->ReleaseBuffer(availableFramesCount), "ReleaseBuffer");
                }

                QCoreApplication::processEvents();
            }
            checkStatus(client->Stop(), "stop client");
            CloseHandle(streamReadyEvent);
            captureClient->Release();
            client->Release();
            stream->deleteLater();
        });

        connect(stream, &Stream::closeMe, this, [endpoint, audioEndpointThread]() {
            endpoint->close();
            audioEndpointThread->wait(QDeadlineTimer(1));
        }, Qt::DirectConnection);
        audioEndpointThread->start();
        break;
    }
    }

    delete[] deviceID;
    device->Release();
    return stream;
}

bool checkStatus(HRESULT result, QString message, std::list<HRESULT> goodStatuses)
{
    if (std::find(goodStatuses.begin(), goodStatuses.end(), result) != goodStatuses.end()) {
        return true;
    }

    _com_error error(result);
    LPCTSTR errorMessage = error.ErrorMessage();
    static const QMap<HRESULT, QString> errorDesc = {
        {AUDCLNT_E_ALREADY_INITIALIZED, "AUDCLNT_E_ALREADY_INITIALIZED"},
        {AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED, "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"},
        {AUDCLNT_E_BUFFER_SIZE_ERROR, "AUDCLNT_E_BUFFER_SIZE_ERROR"},
        {AUDCLNT_E_CPUUSAGE_EXCEEDED, "AUDCLNT_E_CPUUSAGE_EXCEEDED"},
        {AUDCLNT_E_DEVICE_INVALIDATED, "AUDCLNT_E_DEVICE_INVALIDATED"},
        {AUDCLNT_E_DEVICE_IN_USE, "AUDCLNT_E_DEVICE_IN_USE"},
        {AUDCLNT_E_ENDPOINT_CREATE_FAILED, "AUDCLNT_E_ENDPOINT_CREATE_FAILED"},
        {AUDCLNT_E_INVALID_DEVICE_PERIOD, "AUDCLNT_E_INVALID_DEVICE_PERIOD"},
        {AUDCLNT_E_UNSUPPORTED_FORMAT, "AUDCLNT_E_UNSUPPORTED_FORMAT"},
        {AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED, "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"},
        {AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL, "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"},
        {AUDCLNT_E_SERVICE_NOT_RUNNING, "AUDCLNT_E_SERVICE_NOT_RUNNING"},
        {E_INVALIDARG, "E_INVALIDARG"}
    };
    qDebug() << message << Qt::hex << result << QString::fromWCharArray(errorMessage) << errorDesc.value(result, "unknown");

    return false;
}

} // namespace audio
