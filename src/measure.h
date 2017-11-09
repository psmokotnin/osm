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

#include "fouriertransform.h"
#include "deconvolution.h"

class Measure : public Chartable
{
    Q_OBJECT

    //Current sound level
    Q_PROPERTY(float level READ level NOTIFY levelChanged)
    Q_PROPERTY(float referenceLevel READ referenceLevel NOTIFY referenceLevelChanged)

    Q_PROPERTY(unsigned long delay READ delay WRITE setDelay NOTIFY delayChanged)

    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)

    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY referenceChanelChanged)

private:
    QAudioInput* audio;
    QAudioFormat format;
    QTimer *timer;
    int
        _chanelCount = 2,
        _dataChanel = 0,
        _referenceChanel = 1,
        _average = 0, _setAverage = 0;
    unsigned long _delay = 0;
    int _avgcounter = 0;
    bool _polarity = false;
    bool _averageMedian = false;

    complex **averageData, **averageReference;
    float **averageDeconvolution, **averageMagnitude;

    FourierTransform *dataFT;
    Deconvolution *deconv;
    int newDataCount = 0;

    float _level         = 0.0,
         _referenceLevel = 0.0;

    void averaging();
    void averageRealloc();

protected:
    int fftPower;

public:
    explicit Measure(QObject *parent = nullptr);
    ~Measure();

    void setActive(bool active);

    unsigned int dataChanel() {return _dataChanel;}
    void setDataChanel(unsigned int n) {_dataChanel = n;}

    unsigned int referenceChanel(){return _referenceChanel;}
    void setReferenceChanel(unsigned int n) {_referenceChanel = n;}

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
//    void doubleTFChanged();
    void dataChanelChanged();
    void referenceChanelChanged();

public slots:
    void transform();
    QObject *store();
};

#endif // MEASURE_H
