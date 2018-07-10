#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H
#include <QIODevice>

class InputDevice : public QIODevice
{
    Q_OBJECT

public:
    InputDevice(QObject *parent);

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

signals:
    void recived(const char *data, qint64 len);
};

#endif // INPUTDEVICE_H
