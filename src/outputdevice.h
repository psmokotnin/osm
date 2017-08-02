#ifndef OUTPUTDEVICE_H
#define OUTPUTDEVICE_H

#include <QIODevice>
#include <QDebug>

#include "sample.h"

class OutputDevice : public QIODevice
{
    Q_OBJECT

protected:
    int sampleRate;

public:
    OutputDevice(QObject *parent);
    QString name = "Silent";

    qint64 writeData(const char *data, qint64 len);
    qint64 readData(char *data, qint64 maxlen);
    virtual Sample sample(void);

    void setSamplerate(int s) {sampleRate = s;};

};

#endif // OUTPUTDEVICE_H
