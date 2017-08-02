#include "source.h"

Source::Source(QObject *parent) : QIODevice(parent)
{
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    audio = new QAudioInput(format, this);
    open(WriteOnly);
    audio->start(this);
}

qint64 Source::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);

    return -1;
}

qint64 Source::writeData(const char *data, qint64 len)
{
    Sample s;

    for (qint64 i = 0; i < len; i += 4){
        s.c[0] = data[i];
        s.c[1] = data[i + 1];
        s.c[2] = data[i + 2];
        s.c[3] = data[i + 3];
    }
    //qDebug() << s.f << " " << len;

    return len;
}
