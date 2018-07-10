#include "inputdevice.h"
#include <qdebug.h>
InputDevice::InputDevice(QObject *parent) : QIODevice(parent)
{

}
qint64 InputDevice::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}
qint64 InputDevice::writeData(const char *data, qint64 len)
{
    emit recived(data, len);
    return len;
}
