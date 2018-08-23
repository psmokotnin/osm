#ifndef MEASURE_H
#define MEASURE_H

#include <QObject>
#include <QAudioInput>

#include <QTimer>
#include <QtCharts/QAbstractSeries>
QT_CHARTS_USE_NAMESPACE

#include "sample.h"
#include "audiostack.h"
#include "meter.h"
#include "chart/type.h"
#include "chart/source.h"
#include "stored.h"
#include "inputdevice.h"
#include "fouriertransform.h"
#include "deconvolution.h"
#include "filter.h"

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
    Q_PROPERTY(long estimated READ estimated NOTIFY estimatedChanged)

    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(bool lpf READ lpf WRITE setLPF NOTIFY lpfChanged)

    //routing
    Q_PROPERTY(int chanelsCount READ chanelsCount NOTIFY chanelsCountChanged)
    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY referenceChanelChanged)

    //data window type
    Q_PROPERTY(int window READ getWindowType WRITE setWindowType NOTIFY windowTypeChanged)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

private:
    InputDevice *_iodevice;
    QAudioInput* _audio = nullptr;
    QAudioDeviceInfo _device;
    QAudioFormat _format;
    QTimer *_timer;
    unsigned int
        _chanelCount = 2,   //how many chanels will be presented in writeData
        _maxChanelCount = 2,//max supported channels by selected device
        _dataChanel = 0,
        _referenceChanel = 1,
        _average = 0, _setAverage = 0;
    unsigned long _delay = 0;
    long _estimatedDelay = 0;
    unsigned int _avgcounter = 0;
    bool _polarity          = false;
    bool _averageMedian     = false;
    bool _lpf               = true;

    AudioStack *dataStack,
               *referenceStack;
    complex *impulseData;
    Meter dataMeter, referenceMeter;

    complex **averageData = nullptr, **averageReference = nullptr;
    float **averageDeconvolution = nullptr;
    unsigned long *estimatedDelays = nullptr;

    WindowFunction *_window;
    FourierTransform *_dataFT;
    Deconvolution *_deconv;
    Filter *dataLPFs = nullptr, *referenceLPFs = nullptr;

    float _level         = 0.0,
         _referenceLevel = 0.0;

    void calculateDataLength();
    void averaging();
    void averageRealloc(bool force = false);

protected:
    unsigned int _fftPower;

public:
    explicit Measure(QObject *parent = nullptr);
    ~Measure();

    unsigned int fftPower() {return _fftPower;}
    void setFftPower(unsigned int power);

    void setActive(bool active);

    QVariant getDeviceList(void);
    void setDevice(QString deviceName);

    QString deviceName();
    void selectDevice(QString name);

    unsigned int dataChanel() {return _dataChanel;}
    void setDataChanel(unsigned int n) {_dataChanel = n;}

    unsigned int referenceChanel(){return _referenceChanel;}
    void setReferenceChanel(unsigned int n) {_referenceChanel = n;}
    unsigned int chanelsCount() {return _maxChanelCount;}

    float level() {return _level;}
    float referenceLevel() {return _referenceLevel;}

    unsigned long delay() {return _delay;}
    void setDelay(unsigned long delay);

    unsigned int average() {return _average;}
    void setAverage(unsigned int average);

    bool polarity() {return _polarity;}
    void setPolarity(bool polarity) {_polarity = polarity;}

    bool lpf() {return _lpf;}
    void setLPF(bool lpf) {_lpf = lpf;}

    unsigned int sampleRate() const;

    QVariant getAvailableWindowTypes() {return _window->getTypes();}
    int getWindowType() {return static_cast<int>(_window->type());}
    void setWindowType(int t) {_window->setType(static_cast<WindowFunction::Type>(t));}

    QTimer *getTimer() const;
    void setTimer(QTimer *value);

    long estimated() const noexcept;

signals:
    void fftPowerChanged();
    void deviceChanged();
    void levelChanged();
    void delayChanged();
    void referenceLevelChanged();
    void averageChanged();
    void polarityChanged();
    void dataChanelChanged();
    void referenceChanelChanged();
    void windowTypeChanged();
    void estimatedChanged();
    void lpfChanged();
    void chanelsCountChanged();

public slots:
    void transform();
    QObject *store();
    qint64 writeData(const char *_ftdata, qint64 len);
};

#endif // MEASURE_H
