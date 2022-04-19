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

#include "meta/metameasurement.h"
#include "audio/deviceinfo.h"
#include "audio/stream.h"
#include "inputdevice.h"
#include "chart/type.h"
#include "chart/source.h"
#include "stored.h"
#include "math/meter.h"
#include "math/averaging.h"
#include "math/fouriertransform.h"
#include "math/deconvolution.h"
#include "math/filter.h"
#include "math/coherence.h"
#include "common/settings.h"
#include "container/circular.h"

class Measurement : public chart::Source, public meta::Measurement
{
    Q_OBJECT

    //meta properties
    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY
               referenceChanelChanged)
    Q_PROPERTY(unsigned int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)
    Q_PROPERTY(meta::Measurement::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(meta::Measurement::AverageType averageType READ averageType WRITE setAverageType NOTIFY averageTypeChanged)
    Q_PROPERTY(Filter::Frequency filtersFrequency READ filtersFrequency WRITE setFiltersFrequency NOTIFY
               filtersFrequencyChanged)
    Q_PROPERTY(WindowFunction::Type window READ windowFunctionType WRITE setWindowFunctionType NOTIFY
               windowFunctionTypeChanged)

    //constant meta properties
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

    //local properties
    Q_PROPERTY(int sampleRate READ sampleRate NOTIFY audioFormatChanged)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged REVISION NO_API_REVISION)

    //Current sound level
    Q_PROPERTY(float level READ level NOTIFY levelChanged REVISION NO_API_REVISION)
    Q_PROPERTY(float referenceLevel READ referenceLevel NOTIFY referenceLevelChanged REVISION NO_API_REVISION)

    Q_PROPERTY(float measurementPeak READ measurementPeak NOTIFY levelChanged REVISION NO_API_REVISION)
    Q_PROPERTY(float referencePeak READ referencePeak NOTIFY referenceLevelChanged REVISION NO_API_REVISION)

    Q_PROPERTY(long estimated READ estimated NOTIFY estimatedChanged)
    Q_PROPERTY(long estimatedDelta READ estimatedDelta NOTIFY estimatedChanged)

    Q_PROPERTY(bool error MEMBER m_error NOTIFY errorChanged)

    //calibration
    Q_PROPERTY(bool calibrationLoaded READ calibrationLoaded NOTIFY calibrationLoadedChanged)
    Q_PROPERTY(bool calibration READ calibration WRITE setCalibration NOTIFY calibrationChanged)

public:
    explicit Measurement(Settings *settings = nullptr, QObject *parent = nullptr);
    ~Measurement() override;
    Source *clone() const override;

    void setActive(bool active) override;

    Q_INVOKABLE QJsonObject toJSON(const SourceList * = nullptr) const noexcept override;
    void fromJSON(QJsonObject data, const SourceList * = nullptr) noexcept override;

    float level() const;
    float referenceLevel() const;

    float measurementPeak() const;
    float referencePeak() const;

    Q_INVOKABLE void resetAverage() noexcept override;
    Q_INVOKABLE chart::Source *store() noexcept override;

    unsigned int sampleRate() const;

    long estimated() const noexcept;
    long estimatedDelta() const noexcept;

    bool calibration() const noexcept;
    bool calibrationLoaded() const noexcept;
    void setCalibration(bool c) noexcept;
    Q_INVOKABLE bool loadCalibrationFile(const QUrl &fileName) noexcept;

    audio::DeviceInfo::Id deviceId() const;
    void setDeviceId(const audio::DeviceInfo::Id &deviceId);
    QString deviceName() const;
    void selectDevice(const QString &name);

public slots:
    void transform();
    void onSampleRateChanged();
    void writeData(const char *data, qint64 len);
    void setError();
    void newSampleFromGenerator(float sample);
    void resetLoopBuffer();

protected slots:
    void updateFftPower();
    void updateDelay();
    void updateAverage();
    void updateWindowFunction();
    void updateFilterFrequency();

private:
    QTimer m_timer;
    QThread m_timerThread;
    InputDevice m_input;

    audio::DeviceInfo::Id m_deviceId;
    audio::Stream *m_audioStream;

    Settings *m_settings;
    Mode m_currentMode;

    bool m_resetDelay;
    unsigned int m_workingDelay, m_delayFinderCounter;
    long m_estimatedDelay;
    bool m_error;

    container::circular<float> m_data, m_reference, m_loopBuffer;
    Meter m_dataMeter, m_referenceMeter;

    FourierTransform m_dataFT;
    Deconvolution m_deconvolution, m_delayFinder;

    Averaging<float> m_deconvAvg;
    Averaging<float> m_magnitudeAvg, m_moduleAvg;
    Averaging<complex> m_pahseAvg;
    Coherence m_coherence;

    container::array<Filter::BesselLPF<float>> m_moduleLPFs, m_magnitudeLPFs, m_deconvLPFs;
    container::array<Filter::BesselLPF<complex>> m_phaseLPFs;
    container::array<Meter> m_meters;

    void calculateDataLength();
    void averaging();

    bool m_enableCalibration, m_calibrationLoaded;
    QList<QVector<float>> m_calibrationList;
    QVector<float> m_calibrationGain;
    QVector<float> m_calibrationPhase;
    void applyCalibration();

    void updateAudio();
    void checkChannels();

signals:
    void audioFormatChanged();
    void deviceIdChanged(audio::DeviceInfo::Id);
    void deviceNameChanged(QString);
    void levelChanged();
    void referenceLevelChanged();
    void estimatedChanged();
    void errorChanged(bool);
    void calibrationChanged(bool);
    void calibrationLoadedChanged(bool);

    void polarityChanged(bool) override;
    void gainChanged(float) override;
    void modeChanged(meta::Measurement::Mode) override;
    void averageChanged(unsigned int) override;
    void dataChanelChanged(unsigned int) override;
    void referenceChanelChanged(unsigned int) override;
    void averageTypeChanged(meta::Measurement::AverageType) override;
    void windowFunctionTypeChanged(WindowFunction::Type) override;
    void filtersFrequencyChanged(Filter::Frequency) override;
    void delayChanged(unsigned int) override;
    void sampleRateChanged(unsigned int) override;
};

#endif // MEASUREMENT_H
