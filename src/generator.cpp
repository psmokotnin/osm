#include "generator.h"

Generator::Generator(QObject *parent) : QObject(parent)
{
    _sources << new PinkNoise(this);
    _sources << new WhiteNoise(this);
    _sources << new SinNoise(this);

    _device = QAudioDeviceInfo::defaultOutputDevice();
    // Set up the format, eg.
    _format.setSampleRate(48000);
    _format.setChannelCount(1);
    _format.setSampleSize(32);
    _format.setCodec("audio/pcm");
    _format.setByteOrder(QAudioFormat::LittleEndian);
    _format.setSampleType(QAudioFormat::Float);

    _audio = new QAudioOutput(_device, _format, this);
    _audio->setBufferSize(16384);
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
        if (_sources[type]->openMode() == QIODevice::NotOpen)
            _sources[type]->open(QIODevice::ReadOnly);

        _sources[type]->setSamplerate(_format.sampleRate());
        _audio->start(_sources[type]);
    }
    else
        _audio->stop();
}

QVariant Generator::getAvailableTypes(void)
{
    QStringList nameList;
    foreach (OutputDevice* o, _sources) {
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
QString Generator::deviceName()
{
    return _device.deviceName();
}
void Generator::selectDevice(QString name)
{
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        if (name == deviceInfo.deviceName()) {
            _device = deviceInfo;

            _audio->stop();
            _sources[type]->close();
            delete _audio;
            _audio = new QAudioOutput(_device, _format, this);
            _audio->setBufferSize(16384);

            if (enabled) {
                _sources[type]->open(QIODevice::ReadOnly);
                _sources[type]->setSamplerate(_format.sampleRate());
                _audio->start(_sources[type]);
            }
        }
    }
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
