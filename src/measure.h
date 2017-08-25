#ifndef MEASURE_H
#define MEASURE_H

#include <QObject>
#include <QAudioInput>

#include <QTimer>
#include <QtCharts/QAbstractSeries>
QT_CHARTS_USE_NAMESPACE

#include "sample.h"
#include "chartable.h"
#include "stored.h"

class Measure : public Chartable
{
    Q_OBJECT

    //Current sound level
    Q_PROPERTY(float level READ level NOTIFY levelChanged)
    Q_PROPERTY(float referenceLevel READ referenceLevel NOTIFY referenceLevelChanged)

    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)

private:
    QAudioInput* audio;
    QAudioFormat format;
    QTimer *timer;
    int
        _chanelCount = 2,
        _dataChanel = 1,
        _referenceChanel = 0,
        _delay = 0;

    FFT *fft;
    complex *workingData, *workingReferenceData, *workingImpulseData;

    float _level         = 0.0,
         _referenceLevel = 0.0;

protected:
    int fftPower;
    AudioStack *delayStack;

public:
    explicit Measure(QObject *parent = nullptr);

    void setActive(bool active);

    float level() {return _level;}
    float referenceLevel() {return _referenceLevel;}

    int delay(){return _delay;}
    void setDelay(int delay);

    int sampleRate();

    //IO methods
    qint64 writeData(const char *data, qint64 len);

signals:
    void readyRead();
    void levelChanged();
    void delayChanged();
    void referenceLevelChanged();

public slots:
    void transform();
    QObject *store();
};

#endif // MEASURE_H
