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
#include "averaging.h"
#include "fouriertransform.h"
#include "deconvolution.h"
#include "filter.h"
#include "measurementaudiothread.h"

class Measurement : public Fftchart::Source
{
    Q_OBJECT

    //fft
    Q_PROPERTY(int fftPower READ fftPower WRITE setFftPower NOTIFY fftPowerChanged)
    //Current Audio device sample rate
    Q_PROPERTY(int sampleRate READ sampleRate NOTIFY sampleRateChanged)

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
    QTimer m_timer;
    QThread m_timerThread;
    MeasurementAudioThread m_audioThread;
    unsigned int m_average;
    unsigned long m_delay, m_setDelay;
    long m_estimatedDelay;
    bool m_polarity, m_lpf;

    AudioStack *dataStack,
               *referenceStack;
    Meter dataMeter, referenceMeter;

    WindowFunction m_window;
    FourierTransform m_dataFT;
    Deconvolution m_deconvolution;
    Averaging<float> deconvAvg;
    Averaging<unsigned int> estimatedDelayAvg;

    container::array<Filter<float>> m_moduleLPFs, m_magnitudeLPFs, m_deconvLPFs;
    container::array<Filter<complex>> m_phaseLPFs;
    Averaging<float> magnitudeAvg, moduleAvg;
    Averaging<complex> pahseAvg;

    void calculateDataLength();
    void averaging();

//    int m_sampleRate;

protected:
    unsigned int _fftPower, _setfftPower;
    void updateFftPower();
    void updateDelay();

public:
    explicit Measurement(QObject *parent = nullptr);
    ~Measurement();

    unsigned int fftPower() const {return _fftPower;}
    void setFftPower(unsigned int power);

    void setActive(bool active);

    QVariant getDeviceList() const;
    void setDevice(QString deviceName);

    QString deviceName() const;
    void selectDevice(const QString &name);
    void selectDevice(const QAudioDeviceInfo &deviceInfo);

    unsigned int dataChanel() const {return m_audioThread.dataChanel();}
    void setDataChanel(unsigned int n) {m_audioThread.setDataChanel(n);}

    unsigned int referenceChanel() const {return m_audioThread.referenceChanel();}
    void setReferenceChanel(unsigned int n) {m_audioThread.setReferenceChanel(n);}
    unsigned int chanelsCount() const {return m_audioThread.chanelsCount();}

    float level() const {return dataMeter.value();}
    float referenceLevel() const {return referenceMeter.value();}

    unsigned long delay() const {return m_delay;}
    void setDelay(unsigned long delay);

    unsigned int average() const {return m_average;}
    void setAverage(unsigned int average);

    bool polarity() const {return m_polarity;}
    void setPolarity(bool polarity) {m_polarity = polarity;}

    bool lpf() const {return m_lpf;}
    void setLPF(bool lpf) {m_lpf = lpf;}

    unsigned int sampleRate() const;

    QVariant getAvailableWindowTypes() const {return m_window.getTypes();}
    int getWindowType() const {return static_cast<int>(m_window.type());}
    void setWindowType(int t);

    long estimated() const noexcept;

signals:
    void fftPowerChanged(unsigned int power);
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

    void sampleRateChanged(int sampleRate);

public slots:
    void transform();
    void recalculateDataLength();
    QObject *store();
    void writeData(const QByteArray& buffer);
//    void setSampleRate(int sampleRate)
//    {
//        if (m_sampleRate == sampleRate)
//            return;

//        m_sampleRate = sampleRate;
//        emit sampleRateChanged(m_sampleRate);
//    }
};

#endif // MEASUREMENT_H
