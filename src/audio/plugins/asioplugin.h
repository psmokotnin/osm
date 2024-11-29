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
#ifndef AUDIO_ASIOPLUGIN_H
#define AUDIO_ASIOPLUGIN_H

#include "../plugin.h"
#include <QVector>
#include <QThread>
#include <ginclude.h>
#include <asio.h>
#include <asiodrivers.h>

namespace audio {

namespace asioCallbacks {
void bufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
void sampleRateDidChange(ASIOSampleRate sRate);
long asioMessage(long selector, long value, void *message, double *opt);
ASIOTime *bufferSwitchTimeInfo(ASIOTime *params, long doubleBufferIndex, ASIOBool directProcess);
}

class ASIOPlugin : public Plugin
{
    Q_OBJECT
    friend ASIOTime *asioCallbacks::bufferSwitchTimeInfo(ASIOTime *params, long doubleBufferIndex, ASIOBool directProcess);
    friend long asioCallbacks::asioMessage(long selector, long value, void *message, double *opt);
    friend void asioCallbacks::sampleRateDidChange(ASIOSampleRate sampleRate);

public:
    ASIOPlugin();
    ~ASIOPlugin();

    QString name() const override;
    DeviceInfo::List getDeviceInfoList() const override;
    DeviceInfo::Id defaultDeviceId(const Direction &) const override;

    Format deviceFormat(const DeviceInfo::Id &id, const Direction &mode) const override;
    Stream *open(const DeviceInfo::Id &id, const Direction &mode, const Format &, QIODevice *endpoint) override;

signals:
    void runInputProcessing(const QVector<float> &buffer, QPrivateSignal);

private:
    void loadDeviceList();
    bool startDevice(const DeviceInfo::Id &id);
    void stopCurrentDevice();
    void stopAllStreams();
    DeviceInfo deviceInfo(const DeviceInfo::Id &id) const;

    ASIOTime *processBuffers(ASIOTime *params, long doubleBufferIndex, ASIOBool directProcess);
    void processInputStreams(const QVector<float> &buffer);
    void processOutputStreams();
    void sampleRateDidChange(ASIOSampleRate sRate);

    unsigned int m_bufferSize;
    QThread m_workingThread;
    DeviceInfo::Id m_currentDevice;
    AsioDrivers m_drivers;
    DeviceInfo::List m_deviceList;
    QHash<Stream *, std::function<void(void)>> m_outputCallbacks;
    QHash<Stream *, std::function<void(const QVector<float> &)>> m_inputCallbacks;
    QVector<ASIOBufferInfo> m_bufferInfo;
    QVector<ASIOChannelInfo> m_currentChannelInfo;
    QVector<float> m_inputBuffer, m_outputBuffer;
};
} // namespace audio
#endif // AUDIO_ASIOPLUGIN_H
