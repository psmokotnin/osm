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

#include "sample.h"
#include "meter.h"
#include "chart/type.h"
#include "chart/source.h"
#include "stored.h"
#include "averaging.h"
#include "fouriertransform.h"
#include "deconvolution.h"
#include "filter.h"
#include "measurementaudiothread.h"
#include "coherence.h"
#include "settings.h"

class Measurement : public Fftchart::Source
{
    Q_OBJECT

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)

    Q_PROPERTY(int sampleRate READ sampleRate NOTIFY sampleRateChanged)

    //Available input devices
    Q_PROPERTY(QVariant devices READ getDeviceList CONSTANT)
    Q_PROPERTY(QString device READ deviceName WRITE selectDevice NOTIFY deviceChanged)

    //Current sound level
    Q_PROPERTY(float level READ level NOTIFY levelChanged)
    Q_PROPERTY(float referenceLevel READ referenceLevel NOTIFY referenceLevelChanged)

    Q_PROPERTY(unsigned int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(long estimated READ estimated NOTIFY estimatedChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)

    Q_PROPERTY(AverageType averageType READ averageType WRITE setAverageType NOTIFY averageTypeChanged)
    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)
    Q_PROPERTY(Filter::Frequency filtersFrequency READ filtersFrequency WRITE setFiltersFrequency NOTIFY filtersFrequencyChanged)

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(bool error MEMBER m_error NOTIFY errorChanged)

    //routing
    Q_PROPERTY(int chanelsCount READ chanelsCount NOTIFY chanelsCountChanged)
    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY referenceChanelChanged)

    //data window type
    Q_PROPERTY(WindowFunction::Type window READ getWindowType WRITE setWindowType NOTIFY windowTypeChanged)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

    //calibration
    Q_PROPERTY(bool calibrationLoaded READ calibrationLoaded NOTIFY calibrationLoadedChanged)
    Q_PROPERTY(bool calibration READ calibration WRITE setCalibration NOTIFY calibrationChanged)

public:
    enum AverageType {OFF, LPF, FIFO};
    Q_ENUM(AverageType)
    enum Mode {FFT10, FFT12, FFT14, FFT15, FFT16, LFT};
    Q_ENUM(Mode)

    std::map<Mode, QString> modeMap;
    std::map<Mode, int> FFTsizes;
    Q_ENUM(Filter::Frequency)

private:
    QTimer m_timer;
    QThread m_timerThread;
    MeasurementAudioThread m_audioThread;
    Settings *m_settings;
    Mode m_mode, m_currentMode;
    unsigned int m_average;
    unsigned int m_delay, m_setDelay;
    float m_gain;
    long m_estimatedDelay;
    bool m_polarity, m_error;

    container::fifo<float> data, reference;
    Meter dataMeter, referenceMeter;

    WindowFunction::Type m_windowFunctionType;
    FourierTransform m_dataFT;
    Deconvolution m_deconvolution;

    Averaging<float> deconvAvg;
    AverageType m_averageType;
    Averaging<float> magnitudeAvg, moduleAvg;
    Averaging<complex> pahseAvg;
    Coherence m_coherence;

    Filter::Frequency m_filtersFrequency;
    container::array<Filter::BesselLPF<float>> m_moduleLPFs, m_magnitudeLPFs, m_deconvLPFs;
    container::array<Filter::BesselLPF<complex>> m_phaseLPFs;
    void calculateDataLength();
    void averaging();

    bool m_enableCalibration, m_calibrationLoaded;
    QList<QVector<float>> m_calibrationList;
    QVector<float> m_calibrationGain;
    QVector<float> m_calibrationPhase;
    void applyCalibration();

protected:
    void updateFftPower();
    void updateDelay();

public:
    explicit Measurement(Settings *settings = nullptr, QObject *parent = nullptr);
    ~Measurement() override;
    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data) noexcept override;

    Mode mode() const {return m_mode;}
    void setMode(const Measurement::Mode &mode);
    void setMode(QVariant mode) {setMode(mode.value<Mode>());}
    QVariant getAvailableModes() const;

    void setActive(bool active) override;

    QVariant getDeviceList() const;

    QString deviceName() const;
    bool selectDevice(const QString &name);
    void selectDevice(const QAudioDeviceInfo &deviceInfo);

    unsigned int dataChanel() const {return m_audioThread.dataChanel();}
    void setDataChanel(unsigned int n);

    unsigned int referenceChanel() const {return m_audioThread.referenceChanel();}
    void setReferenceChanel(unsigned int n);
    unsigned int chanelsCount() const {return m_audioThread.chanelsCount();}

    float level() const {return dataMeter.value();}
    float referenceLevel() const {return referenceMeter.value();}

    unsigned int delay() const {return m_delay;}
    void setDelay(unsigned int delay);

    unsigned int average() const {return m_average;}
    void setAverage(unsigned int average);

    bool polarity() const {return m_polarity;}
    void setPolarity(bool polarity);

    Filter::Frequency filtersFrequency() const {return m_filtersFrequency;}
    void setFiltersFrequency(Filter::Frequency frequency);
    void setFiltersFrequency(QVariant frequency) {setFiltersFrequency(static_cast<Filter::Frequency>(frequency.toInt()));}

    AverageType averageType() const {return m_averageType;}
    void setAverageType(AverageType type);
    void setAverageType(QVariant type) {setAverageType(static_cast<AverageType>(type.toInt()));}

    unsigned int sampleRate() const;

    QVariant getAvailableWindowTypes() const {return WindowFunction::getTypes();}
    WindowFunction::Type getWindowType() const {return m_windowFunctionType;}
    void setWindowType(WindowFunction::Type type);
    void setWindowType(QVariant type) {setWindowType(static_cast<WindowFunction::Type>(type.toInt()));}

    long estimated() const noexcept;

    Q_INVOKABLE bool loadCalibrationFile(const QUrl &fileName) noexcept;

    bool calibrationLoaded() const noexcept {return m_calibrationLoaded;}
    bool calibration() const noexcept {return m_enableCalibration;}
    void setCalibration(bool c) noexcept;

    Q_INVOKABLE void resetAverage() noexcept;

    //! return and set gain in dB
    float gain() const;
    void setGain(float gain);

signals:
    void modeChanged(Measurement::Mode);
    void sampleRateChanged();
    void deviceChanged(QString);
    void levelChanged();
    void delayChanged(unsigned int);
    void referenceLevelChanged();
    void averageChanged(unsigned int);
    void polarityChanged(bool);
    void dataChanelChanged(unsigned int);
    void referenceChanelChanged(unsigned int);
    void windowTypeChanged(WindowFunction::Type);
    void estimatedChanged();
    void chanelsCountChanged();
    void averageTypeChanged(Measurement::AverageType);
    void filtersFrequencyChanged(Filter::Frequency);
    void errorChanged(bool);
    void calibrationChanged(bool);
    void calibrationLoadedChanged(bool);
    void gainChanged(float);

public slots:
    void transform();
    void recalculateDataLength();
    QObject *store();
    void writeData(const QByteArray& buffer);
    void setError();
};

#endif // MEASUREMENT_H
