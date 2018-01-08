#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QDebug>
#include <QAudio>
#include <QAudioDeviceInfo>
#include <QAudioOutput>

#include "whitenoise.h"
#include "pinknoise.h"
#include "sinnoise.h"

class Generator : public QObject
{
    Q_OBJECT

    //On-off property
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled NOTIFY enabledChanged)

    //current type
    Q_PROPERTY(int type READ getType WRITE setType NOTIFY typeChanged)

    //Available generator types. Constant list.
    Q_PROPERTY(QVariant types READ getAvailableTypes CONSTANT)

    //Available output devices
    Q_PROPERTY(QVariant devices READ getDeviceList CONSTANT)
    Q_PROPERTY(QString device READ deviceName WRITE selectDevice NOTIFY deviceChanged)

    //Frequency
    Q_PROPERTY(int frequency READ getFrequency WRITE setFrequency NOTIFY frequencyChangedQml)

private:
    bool enabled    = false;
    int type        = 0;
    int frequency   = 1000;

    QAudioFormat _format;
    QAudioOutput* _audio;
    QAudioDeviceInfo _device;
    QList<OutputDevice*> _sources;

protected:
    void updateAudio(void);

public:
    explicit Generator(QObject* parent = nullptr);

    bool getEnabled();
    void setEnabled(bool enable);

    int getType();
    void setType(int t);

    QVariant getAvailableTypes(void);
    QVariant getDeviceList(void);

    QString deviceName();
    void selectDevice(QString name);

    int getFrequency();
    void setFrequency(int f);

signals:
    void enabledChanged();
    void typeChanged();
    void frequencyChanged(int f);
    void frequencyChangedQml();
    void deviceChanged();

public slots:
};

#endif // GENERATOR_H
