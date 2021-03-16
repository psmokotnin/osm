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
#include <QTimer>
#include <QThread>

#include "sample.h"
#include "meter.h"
#include "audio/deviceinfo.h"
#include "audio/stream.h"
#include "inputdevice.h"
#include "chart/type.h"
#include "chart/source.h"
#include "stored.h"
#include "averaging.h"
#include "fouriertransform.h"
#include "deconvolution.h"
#include "filter.h"
#include "coherence.h"
#include "settings.h"

class Measurement : public Fftchart::Source
{
    Q_OBJECT

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)

    Q_PROPERTY(int sampleRate READ sampleRate NOTIFY audioFormatChanged)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)

    //Current sound level
    Q_PROPERTY(float level READ level NOTIFY levelChanged)
    Q_PROPERTY(float referenceLevel READ referenceLevel NOTIFY referenceLevelChanged)

    Q_PROPERTY(unsigned int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(long estimated READ estimated NOTIFY estimatedChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)

    Q_PROPERTY(AverageType averageType READ averageType WRITE setAverageType NOTIFY averageTypeChanged)
    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)
    Q_PROPERTY(Filter::Frequency filtersFrequency READ filtersFrequency WRITE setFiltersFrequency NOTIFY
               filtersFrequencyChanged)

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(bool error MEMBER m_error NOTIFY errorChanged)

    //routing
    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY
               referenceChanelChanged)

    //data window type
    Q_PROPERTY(WindowFunction::Type window READ getWindowType WRITE setWindowType NOTIFY
               windowTypeChanged)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

    //calibration
    Q_PROPERTY(bool calibrationLoaded READ calibrationLoaded NOTIFY calibrationLoadedChanged)
    Q_PROPERTY(bool calibration READ calibration WRITE setCalibration NOTIFY calibrationChanged)

public:
    enum AverageType {Off, LPF, FIFO};
    Q_ENUM(AverageType)

    enum Mode {FFT10, FFT12, FFT14, FFT15, FFT16, LFT};
    Q_ENUM(Mode)
    Q_ENUM(Filter::Frequency)

    explicit Measurement(Settings *settings = nullptr, QObject *parent = nullptr);
    ~Measurement() override;

    void setActive(bool active) override;

    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data) noexcept override;

    Mode mode() const;
    void setMode(const Measurement::Mode &mode);
    void setMode(QVariant mode);
    QVariant getAvailableModes() const;

    unsigned int dataChanel() const;
    void setDataChanel(unsigned int channel);
    unsigned int referenceChanel() const;
    void setReferenceChanel(unsigned int channel);

    float level() const;
    float referenceLevel() const;

    unsigned int delay() const;
    void setDelay(unsigned int delay);

    unsigned int average() const;
    void setAverage(unsigned int average);
    Q_INVOKABLE void resetAverage() noexcept;

    bool polarity() const;
    void setPolarity(bool polarity);

    Filter::Frequency filtersFrequency() const;
    void setFiltersFrequency(Filter::Frequency frequency);
    void setFiltersFrequency(QVariant frequency);

    AverageType averageType() const;
    void setAverageType(AverageType type);
    void setAverageType(QVariant type);

    unsigned int sampleRate() const;

    QVariant getAvailableWindowTypes() const;
    WindowFunction::Type getWindowType() const;
    void setWindowType(WindowFunction::Type type);
    void setWindowType(QVariant type);

    long estimated() const noexcept;

    bool calibration() const noexcept;
    bool calibrationLoaded() const noexcept;
    void setCalibration(bool c) noexcept;
    Q_INVOKABLE bool loadCalibrationFile(const QUrl &fileName) noexcept;

    //! return and set gain in dB
    float gain() const;
    void setGain(float gain);

    audio::DeviceInfo::Id deviceId() const;
    void setDeviceId(const audio::DeviceInfo::Id &deviceId);
    QString deviceName() const;
    void selectDevice(const QString &name);

public slots:
    void transform();
    void recalculateDataLength();
    QObject *store();
    void writeData(const QByteArray &buffer);
    void setError();

protected:
    void updateFftPower();
    void updateDelay();

private:
    QTimer m_timer;
    QThread m_timerThread;
    InputDevice m_input;
    std::map<Mode, QString> m_modeMap;
    std::map<Mode, int> m_FFTsizes;
    audio::DeviceInfo::Id m_deviceId;
    audio::Stream *m_audioStream;

    Settings *m_settings;
    Mode m_mode, m_currentMode;

    unsigned int m_dataChanel, m_referenceChanel;
    unsigned int m_average;
    unsigned int m_delay, m_setDelay;
    float m_gain;
    long m_estimatedDelay;
    bool m_polarity, m_error;

    container::fifo<float> m_data, m_reference;
    Meter m_dataMeter, m_referenceMeter;

    WindowFunction::Type m_windowFunctionType;
    FourierTransform m_dataFT;
    Deconvolution m_deconvolution;

    Averaging<float> m_deconvAvg;
    AverageType m_averageType;
    Averaging<float> m_magnitudeAvg, m_moduleAvg;
    Averaging<complex> m_pahseAvg;
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

    void updateAudio();

signals:
    void modeChanged(Measurement::Mode);
    void audioFormatChanged();
    void deviceIdChanged(audio::DeviceInfo::Id);
    void deviceNameChanged(QString);
    void levelChanged();
    void delayChanged(unsigned int);
    void referenceLevelChanged();
    void averageChanged(unsigned int);
    void polarityChanged(bool);
    void dataChanelChanged(unsigned int);
    void referenceChanelChanged(unsigned int);
    void windowTypeChanged(WindowFunction::Type);
    void estimatedChanged();
    void averageTypeChanged(Measurement::AverageType);
    void filtersFrequencyChanged(Filter::Frequency);
    void errorChanged(bool);
    void calibrationChanged(bool);
    void calibrationLoadedChanged(bool);
    void gainChanged(float);
};

#endif // MEASUREMENT_H
