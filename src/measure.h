#ifndef MEASURE_H
#define MEASURE_H

#include <QObject>
#include <QAudioInput>

#include <QTimer>
#include <QtCharts/QAbstractSeries>
QT_CHARTS_USE_NAMESPACE

#include "sample.h"
#include "audiostack.h"
#include "chart/type.h"
#include "chart/source.h"
#include "stored.h"
#include "inputdevice.h"
#include "fouriertransform.h"
#include "deconvolution.h"

class Measure : public Fftchart::Source
{
    Q_OBJECT

    //fft
    Q_PROPERTY(int fftPower READ fftPower WRITE setFftPower NOTIFY fftPowerChanged)

    //Available input devices
    Q_PROPERTY(QVariant devices READ getDeviceList CONSTANT)
    Q_PROPERTY(QString device READ deviceName WRITE selectDevice NOTIFY deviceChanged)

    //Current sound level
    Q_PROPERTY(float level READ level NOTIFY levelChanged)
    Q_PROPERTY(float referenceLevel READ referenceLevel NOTIFY referenceLevelChanged)

    Q_PROPERTY(unsigned long delay READ delay WRITE setDelay NOTIFY delayChanged)

    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)

    //routing
    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY referenceChanelChanged)

    //data window type
    Q_PROPERTY(int window READ getWindowType WRITE setWindowType NOTIFY windowTypeChanged)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

    //DTW
    Q_PROPERTY(int doubleTW READ doubleTW WRITE setDoubleTW NOTIFY doubleTWChanged)

private:
    InputDevice *_iodevice;
    QAudioInput* _audio;
    QAudioDeviceInfo _device;
    QAudioFormat _format;
    QTimer *_timer;
    int
        _chanelCount = 2,
        _dataChanel = 0,
        _referenceChanel = 1,
        _average = 0, _setAverage = 0;
    unsigned long _delay = 0;
    int _avgcounter = 0;
    bool _polarity = false;
    bool _averageMedian = false;

    AudioStack *dataStack,
               *referenceStack;
    complex *impulseData;

    complex **averageData = nullptr, **averageReference = nullptr;
    float **averageDeconvolution = nullptr;

    WindowFunction *_window;
    FourierTransform *_dataFT;
    Deconvolution *_deconv;

    float _level         = 0.0,
         _referenceLevel = 0.0;

    void calculateDataLength();
    void averaging();
    void averageRealloc(bool force = false);

protected:
    int _fftPower;

public:
    explicit Measure(QObject *parent = nullptr);
    ~Measure();

    int fftPower() {return _fftPower;}
    void setFftPower(int power);

    int fftSize() {return _fftSize;}
    void setFftSize(int size) {_fftSize = size;}

    bool doubleTW() const {return _dataFT->doubleTW();}
    void setDoubleTW(bool doubleTW);

    void setActive(bool active);

    QVariant getDeviceList(void);
    void setDevice(QString deviceName);

    QString deviceName();
    void selectDevice(QString name);

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

    int sampleRate() const;

    //IO methods
    //qint64 writeData(const char *data, qint64 len);

    QVariant getAvailableWindowTypes() {return _window->getTypes();}
    int getWindowType() {return (int)_window->type();}
    void setWindowType(int t) {_window->setType((WindowFunction::Type)t);}

    QTimer *getTimer() const;
    void setTimer(QTimer *value);

signals:
    void fftPowerChanged();
    void deviceChanged();
    void readyRead();
    void levelChanged();
    void delayChanged();
    void referenceLevelChanged();
    void averageChanged();
    void polarityChanged();
    void dataChanelChanged();
    void referenceChanelChanged();
    void windowTypeChanged();
    void doubleTWChanged();

public slots:
    void transform();
    QObject *store();
    qint64 writeData(const char *_ftdata, qint64 len);
};

#endif // MEASURE_H
