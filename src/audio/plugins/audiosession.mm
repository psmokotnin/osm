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
#include "audiosession.h"
#import <AVFoundation/AVFoundation.h>

namespace audio {

bool checkStatus(OSStatus status, QString message = "", std::list<OSStatus> goodStatuses = {noErr});

# define checkCall(foo, returnval, message) \
    if (!checkStatus(foo, message)) {\
    return returnval;\
};

void audioInterruptHandler(void *userData, unsigned int state)
{
    auto plugin = reinterpret_cast<AudioSessionPlugin *>(userData);
    switch (state) {
    case AVAudioSessionInterruptionTypeBegan:
        plugin->stopQueues();
        break;
    case AVAudioSessionInterruptionTypeEnded:
        break;
    }

}
void notificationHandler(CFNotificationCenterRef, void *userData, CFStringRef CFName, const void *,
                         CFDictionaryRef dictionaryRef)
{
    auto plugin = reinterpret_cast<AudioSessionPlugin *>(userData);
    auto name = QString::fromCFString(CFName);

    if (name == "AVAudioSessionRouteChangeNotification") {
        emit plugin->deviceListChanged();
        return;
    }

    if (name == "AVAudioSessionInterruptionNotification") {
        CFNumberRef interruptionTypeRef = NULL;
        interruptionTypeRef = (CFNumberRef)CFDictionaryGetValue(dictionaryRef, AVAudioSessionInterruptionTypeKey);
        int interruptionType;
        if (CFNumberGetValue(interruptionTypeRef, kCFNumberSInt32Type, &interruptionType)) {
            audioInterruptHandler(userData, interruptionType);
        } else {
            audioInterruptHandler(userData, 0);
        }
        return;
    }
}

const DeviceInfo::Id AudioSessionPlugin::OUTPUT_DEVICE_ID = "output";
const DeviceInfo::Id AudioSessionPlugin::INPUT_DEVICE_ID = "input";

AudioSessionPlugin::AudioSessionPlugin() : m_permission(false)
{
    [[AVAudioSession sharedInstance] requestRecordPermission: [this] (bool permission) {
                                        m_permission = permission;
                                    }];
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord withOptions:
                                     AVAudioSessionCategoryOptionMixWithOthers error:nil];

    [[AVAudioSession sharedInstance] setActive:true error:nil];

    CFNotificationCenterAddObserver(
        CFNotificationCenterGetLocalCenter(),
        this,
        &notificationHandler,
        CFSTR("AVAudioSessionRouteChangeNotification"),
        NULL,
        CFNotificationSuspensionBehaviorDeliverImmediately
    );

    CFNotificationCenterAddObserver(
        CFNotificationCenterGetLocalCenter(),
        this,
        &notificationHandler,
        CFSTR("AVAudioSessionInterruptionNotification"),
        NULL,
        CFNotificationSuspensionBehaviorDeliverImmediately
    );

}

AudioSessionPlugin::~AudioSessionPlugin()
{
    [[AVAudioSession sharedInstance] setActive:false error:nil];
}

QString AudioSessionPlugin::name() const
{
    return "AudioSession";
}

DeviceInfo::List AudioSessionPlugin::getDeviceInfoList() const
{
    AVAudioSession *audioSession = AVAudioSession.sharedInstance;
    auto route = audioSession.currentRoute;

    DeviceInfo::List list = {};
    NSEnumerator *inputs = [[route inputs] objectEnumerator];
    while (id object = [inputs nextObject]) {
        DeviceInfo info(INPUT_DEVICE_ID, name());
        info.setName(QString::fromNSString([object portName]));
        info.setDefaultSampleRate([audioSession preferredSampleRate]);

        QStringList channelList;
        NSEnumerator *channels = [[object channels] objectEnumerator];
        while (id channel = [channels nextObject]) {
            channelList << QString::number([channel channelNumber]);//QString::fromNSString([channel channelName]);
        }
        info.setInputChannels(channelList);
        list << info;
    }

    NSEnumerator *outputs = [[route outputs] objectEnumerator];
    while (id object = [outputs nextObject]) {
        DeviceInfo info(OUTPUT_DEVICE_ID, name());
        info.setName(QString::fromNSString([object portName]));
        info.setDefaultSampleRate([audioSession preferredSampleRate]);

        QStringList channelList;
        NSEnumerator *channels = [[object channels] objectEnumerator];
        while (id channel = [channels nextObject]) {
            //QString::fromNSString([channel channelName]);
            channelList << QString::number([channel channelNumber]);
        }
        info.setOutputChannels(channelList);
        list << info;
    }
    return list;
}

DeviceInfo::Id AudioSessionPlugin::defaultDeviceId(const Plugin::Direction &mode) const
{
    switch (mode) {
    case Input:
        return INPUT_DEVICE_ID;
    case Output:
        return OUTPUT_DEVICE_ID;
    }
    return {};
}

Format AudioSessionPlugin::deviceFormat(const DeviceInfo::Id &id, const Plugin::Direction &mode) const
{
    Q_UNUSED(id);

    AVAudioSession *audioSession = AVAudioSession.sharedInstance;
    unsigned int sampleRate = [[AVAudioSession sharedInstance] preferredSampleRate];
    unsigned int channels = (mode == Input ? [audioSession inputNumberOfChannels] : [audioSession outputNumberOfChannels]);
    return {
        sampleRate,
        channels
    };
}

Stream *AudioSessionPlugin::open(const DeviceInfo::Id &, const Plugin::Direction &mode, const Format &format,
                                 QIODevice *endpoint)
{
    AudioStreamBasicDescription streamFormat;
    streamFormat.mSampleRate          = format.sampleRate;
    streamFormat.mFormatID            = kAudioFormatLinearPCM;
    streamFormat.mFormatFlags         = kAudioFormatFlagIsFloat;
    streamFormat.mChannelsPerFrame    = format.channelCount;
    streamFormat.mBitsPerChannel      = 32;
    streamFormat.mReserved            = 0;
    streamFormat.mBytesPerFrame       = streamFormat.mChannelsPerFrame * streamFormat.mBitsPerChannel / 8;
    streamFormat.mFramesPerPacket     = 1;
    streamFormat.mBytesPerPacket      = streamFormat.mBytesPerFrame;
    auto bufferSize                   = streamFormat.mChannelsPerFrame * 8192 * sizeof(float);

    AudioQueueRef queue;
    AudioQueueBufferRef buffers[3];

    auto outputCallback = [](void *inUserData, AudioQueueRef queue, AudioQueueBufferRef inBuffer) -> void {
        auto endpoint = reinterpret_cast<QIODevice *>(inUserData);
        if (endpoint && endpoint->isReadable())
        {
            endpoint->read(reinterpret_cast<char *>(inBuffer->mAudioData), inBuffer->mAudioDataByteSize);
            checkStatus(AudioQueueEnqueueBuffer(queue, inBuffer, 0, NULL), "AudioQueueEnqueueBuffer", {noErr, kAudioQueueErr_EnqueueDuringReset});
        }
    };

    auto inputCallback = [](void *inUserData, AudioQueueRef queue, AudioQueueBufferRef inBuffer,
                            const AudioTimeStamp *, UInt32, const AudioStreamPacketDescription *)
    -> void {
        auto endpoint = reinterpret_cast<QIODevice *>(inUserData);
        if (endpoint && endpoint->isWritable())
        {
            endpoint->write(reinterpret_cast<char *>(inBuffer->mAudioData), inBuffer->mAudioDataByteSize);
            checkStatus(AudioQueueEnqueueBuffer(queue, inBuffer, 0, NULL), "AudioQueueEnqueueBuffer", {noErr, kAudioQueueErr_EnqueueDuringReset});
        }
    };

    switch (mode) {
    case Output: {
        if (endpoint->openMode() == QIODevice::NotOpen) {
            endpoint->open(QIODevice::ReadOnly);
        }
        checkCall(AudioQueueNewOutput(&streamFormat, outputCallback, endpoint, NULL, NULL, 0, &queue),
                  nullptr,
                  "AudioQueueNewOutput");
        break;
    }
    case Input:
        if (endpoint->openMode() == QIODevice::NotOpen) {
            endpoint->open(QIODevice::WriteOnly);
        }
        checkCall(AudioQueueNewInput(&streamFormat, inputCallback, endpoint, NULL, NULL, 0, &queue),
                  nullptr,
                  "AudioQueueNewInput");
        break;
    }

    for (int i = 0; i < 3; i++) {
        checkCall(AudioQueueAllocateBuffer(queue, bufferSize, &buffers[i]),
                  nullptr,
                  "AudioQueueAllocateBuffer");
        buffers[i]->mAudioDataByteSize = bufferSize;
        switch (mode) {

        case Output:
            outputCallback(endpoint, queue, buffers[i]);
            break;
        case Input:
            inputCallback(endpoint, queue, buffers[i], nullptr, 0, nullptr);
            break;
        }
    }
    auto res = AudioQueueStart(queue, NULL);
    if (!checkStatus(res, "start output")) {
        AudioQueueDispose(queue, true);
        return nullptr;
    }

    auto stream = new Stream(format);
    connect(stream, &Stream::closeMe, this, [queue, endpoint]() {
        endpoint->close();
        AudioQueueStop(queue, true);
        AudioQueueDispose(queue, false);
    });
    connect(this, &AudioSessionPlugin::stopStreams, this, [stream]() {
        stream->close();
    }, Qt::DirectConnection);
    return stream;
}

void AudioSessionPlugin::stopQueues()
{
    emit stopStreams({});
}

bool checkStatus(OSStatus status, QString message, std::list<OSStatus> goodStatuses)
{
    if (std::find(goodStatuses.begin(), goodStatuses.end(), status) != goodStatuses.end()) {
        return true;
    }
    union {
        char name[5];
        uint32_t code;
    } error;

    error.code = CFSwapInt32HostToBig(status);
    if (isprint(error.name[0]) && isprint(error.name[1]) && isprint(error.name[2]) && isprint(error.name[3])) {
        error.name[4] = '\0';
        qCritical() << "coreaudio error: '" << error.name << "' " << message;
    } else {
        qCritical() << "coreaudio error: " << status << message;
    }
    return false;
}
} // namespace audio
