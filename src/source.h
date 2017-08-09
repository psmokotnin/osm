#ifndef SOURCE_H
#define SOURCE_H

#include <QIODevice>
#include <QAudioInput>
#include <QQueue>

#include <QDebug>

#include "sample.h"

class Source : public QIODevice
{
    Q_OBJECT

private:
    QAudioInput* audio;
    QAudioFormat format;

    QQueue<Sample> internalBuffer;

protected:

public:
    Source(QObject *parent = nullptr);
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    const int BUFFER_SIZE = 1024;
    QQueue<Sample> buffer;

    int sampleRate();

signals:
    void readyRead();

public slots:
};

#endif // SOURCE_H
