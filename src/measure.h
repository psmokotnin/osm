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

    Q_PROPERTY(unsigned long delay READ delay WRITE setDelay NOTIFY delayChanged)

    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)
    Q_PROPERTY(bool averageMedian READ averageMedian WRITE setAverageMedian NOTIFY averageMedianChanged)

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)


    Q_PROPERTY(bool doubleTF READ doubleTF WRITE setDoubleTF NOTIFY doubleTFChanged)
    bool _doubleTF = false;
    bool doubleTF() {return _doubleTF;}
    void setDoubleTF(bool doubleTF);


private:
    QAudioInput* audio;
    QAudioFormat format;
    QTimer *timer;
    int
        _chanelCount = 2,
        _dataChanel = 1,
        _referenceChanel = 0,
        _average = 0, _setAverage = 0;
    unsigned long _delay = 0;
    int _avgcounter = 0;
    bool _polarity = false;
    bool _averageMedian = false;

    FFT *fft;
    complex *workingData, *workingReferenceData, *workingImpulseData;
    complex **averageImpulseData;
    qreal **averageModule, **averageMagnitude, **averagePhase;

    AudioStack *subDataStack, *subReferenceStack;
    complex *subWorkingData, *subWorkingReferenceData;

    float _level         = 0.0,
         _referenceLevel = 0.0;

    void averaging();
    void averageRealloc();
    void medianAveraging();

protected:
    int fftPower;

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

    bool averageMedian() {return _averageMedian;}
    void setAverageMedian(bool averageMedian) {_averageMedian = averageMedian;}

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
    void averageMedianChanged();
    void doubleTFChanged();

public slots:
    void transform();
    QObject *store();
};

#endif // MEASURE_H
