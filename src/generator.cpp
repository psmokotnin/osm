#include "generator.h"

Generator::Generator(QObject *parent) : QObject(parent)
{
    sources << new PinkNoise(this);
    sources << new WhiteNoise(this);
    sources << new SinNoise(this);

    // Set up the format, eg.
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    audio = new QAudioOutput(format, this);
    audio->setBufferSize(16384);
    //connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
}
bool Generator::getEnabled()
{
    return enabled;
}
void Generator::setEnabled(bool enable)
{
    if (enabled != enable) {
        enabled = enable;
        updateAudio();
    }
}
int Generator::getType()
{
    return type;
}
void Generator::setType(int t)
{
    type = t;

    updateAudio();
}
void Generator::updateAudio(void)
{
    if (enabled) {
        if (sources[type]->openMode() == QIODevice::NotOpen)
            sources[type]->open(QIODevice::ReadOnly);

        sources[type]->setSamplerate(format.sampleRate());
        audio->start(sources[type]);
    }
    else
        audio->stop();
}

QVariant Generator::getAvailableTypes(void)
{
    QStringList nameList;
    foreach (OutputDevice* o, sources) {
        nameList << o->name;
    }
    return QVariant::fromValue(nameList);
}
QVariant Generator::getDeviceList(void)
{
    QStringList deviceList;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        deviceList << deviceInfo.deviceName();
    }
    return QVariant::fromValue(deviceList);
}
int Generator::getFrequency()
{
    return frequency;
}
void Generator::setFrequency(int f)
{
    frequency = f;
    emit frequencyChanged(frequency);
}
