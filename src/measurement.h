/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEASUREMENT_H
#define MEASUREMENT_H

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
#include "measurementaudiothread.h"

class Measurement : public Fftchart::Source
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
    QTimer *_timer;
    QThread m_timerThread;
    MeasurementAudioThread m_audioThread;
    unsigned int _average = 0, _setAverage = 0;
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
    explicit Measurement(QObject *parent = nullptr);
    ~Measurement();

    unsigned int fftPower() {return _fftPower;}
    void setFftPower(unsigned int power);

    void setActive(bool active);

    QVariant getDeviceList(void);
    void setDevice(QString deviceName);

    QString deviceName() const;
    void selectDevice(QString name);
    void selectDevice(QAudioDeviceInfo deviceInfo);

    unsigned int dataChanel() const {return m_audioThread.dataChanel();}
    void setDataChanel(unsigned int n) {m_audioThread.setDataChanel(n);}

    unsigned int referenceChanel() const {return m_audioThread.referenceChanel();}
    void setReferenceChanel(unsigned int n) {m_audioThread.setReferenceChanel(n);}
    unsigned int chanelsCount() const {return m_audioThread.chanelsCount();}

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
    void recalculateDataLength();
    QObject *store();
    qint64 writeData(const char *_ftdata, qint64 len);
};

#endif // MEASUREMENT_H
