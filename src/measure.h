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

    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)

private:
    QAudioInput* audio;
    QAudioFormat format;
    QTimer *timer;
    int
        _chanelCount = 2,
        _dataChanel = 1,
        _referenceChanel = 0,
        _delay = 0,
        _average = 0, _setAverage = 0;
    int _avgcounter = 0;
    bool _polarity = false;

    FFT *fft;
    complex *workingData, *workingReferenceData, *workingImpulseData;
    complex **averageData, **averageReferenceData, **averageImpulseData;

    float _level         = 0.0,
         _referenceLevel = 0.0;

    void averaging();
    void averageRealloc();

protected:
    int fftPower;
    AudioStack *delayStack;

public:
    explicit Measure(QObject *parent = nullptr);
    ~Measure();

    void setActive(bool active);

    float level() {return _level;}
    float referenceLevel() {return _referenceLevel;}

    int delay(){return _delay;}
    void setDelay(int delay);

    int average(){return _average;}
    void setAverage(int average);

    bool polarity() {return _polarity;}
    void setPolarity(bool polarity) {_polarity = polarity;}

    int sampleRate();

    //IO methods
    qint64 writeData(const char *data, qint64 len);

signals:
    void readyRead();
    void levelChanged();
    void delayChanged();
    void referenceLevelChanged();
    void averageChanged();
    void polarityChanged();

public slots:
    void transform();
    QObject *store();
};

#endif // MEASURE_H
