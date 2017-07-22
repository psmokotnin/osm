#include "outputdevice.h"

OutputDevice::OutputDevice(QObject *parent) : QIODevice(parent)
{

}

/**
 * @brief OutputDevice::readData makes silent output stream by default.
 * @param data
 * @param maxlen
 * @return
 */
/*qint64 OutputDevice::readData(char *data, qint64 maxlen)
{
    for (qint64 i = 0; i < maxlen; i++) {
        data[i] = 0;
    }
    return maxlen;
}*/
qint64 OutputDevice::readData(char *data, qint64 maxlen)
{
    OutputDevice::Sample s;

    for (qint64 i = 0; i < maxlen; i += 4) {
        s = this->sample();

        data[i + 0] = s.c[0];
        data[i + 1] = s.c[1];
        data[i + 2] = s.c[2];
        data[i + 3] = s.c[3];
    }
    return maxlen;
}
OutputDevice::Sample OutputDevice::sample(void)
{
    OutputDevice::Sample output;
    output.f = 0.0;
    return output;
}

qint64 OutputDevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return -1;
}
