#ifndef SOURCE_H
#define SOURCE_H

#include <QIODevice>
#include <QAudioInput>
#include <QDebug>

#include "sample.h"

class Source : public QIODevice
{
    Q_OBJECT

private:
    QAudioInput* audio;
    QAudioFormat format;

protected:

public:
    Source(QObject *parent = nullptr);
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

signals:

public slots:
};

#endif // SOURCE_H
