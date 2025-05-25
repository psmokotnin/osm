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
#include "abstract/source.h"
#include "stored.h"
#include "math/meter.h"
#include "math/averaging.h"
#include "math/fouriertransform.h"
#include "math/deconvolution.h"
#include "math/bessellpf.h"
#include "math/coherence.h"
#include "math/filter.h"
#include "common/settings.h"
#include "container/circular.h"

class Measurement : public Abstract::Source, public Meta::Measurement
{
    Q_OBJECT

    Q_PROPERTY(bool polarity READ polarity WRITE setPolarity NOTIFY polarityChanged)
    //meta properties
    Q_PROPERTY(float gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(float offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(int dataChanel READ dataChanel WRITE setDataChanel NOTIFY dataChanelChanged)
    Q_PROPERTY(int referenceChanel READ referenceChanel WRITE setReferenceChanel NOTIFY
               referenceChanelChanged)
    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(int average READ average WRITE setAverage NOTIFY averageChanged)
    Q_PROPERTY(Meta::Measurement::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(Meta::Measurement::AverageType averageType READ averageType WRITE setAverageType NOTIFY averageTypeChanged)
    Q_PROPERTY(Filter::Frequency filtersFrequency READ filtersFrequency WRITE setFiltersFrequency NOTIFY
               filtersFrequencyChanged)
    Q_PROPERTY(WindowFunction::Type window READ windowFunctionType WRITE setWindowFunctionType NOTIFY
               windowFunctionTypeChanged)

    //constant meta properties
    Q_PROPERTY(QVariant modes READ getAvailableModes CONSTANT)
    Q_PROPERTY(QVariant inputFilters READ getAvailableInputFilters CONSTANT)
    Q_PROPERTY(QVariant windows READ getAvailableWindowTypes CONSTANT)

    //local properties
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

    Q_PROPERTY(Meta::Measurement::InputFilter inputFilter READ inputFilter WRITE setInputFilter NOTIFY inputFilterChanged)

public:
    explicit Measurement(QObject *parent = nullptr);
    ~Measurement() override;

    static const unsigned int TIMER_INTERVAL = 80; //ms = 12.5 per sec

    Shared::Source clone() const override;

    void setActive(bool newActive) final;

    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

    float level(const Weighting::Curve curve = Weighting::Z, const Meter::Time time = Meter::Fast) const override;
    float peak(const Weighting::Curve curve = Weighting::Z, const Meter::Time time = Meter::Fast) const override;
    float referenceLevel() const override;

    float measurementPeak() const;
    float referencePeak() const;

    Q_INVOKABLE void resetAverage() noexcept override;
    Q_INVOKABLE Shared::Source store() override;

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

    Q_INVOKABLE void applyAutoGain(const float reference) override;
    Q_INVOKABLE void destroy() override final;

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
    void applyInputFilters();

private:
    QTimer m_timer;
    QThread m_timerThread;
    InputDevice m_input;

    audio::DeviceInfo::Id m_deviceId;
    audio::Stream *m_audioStream;

    Settings *m_settings;
    Mode m_currentMode;

    bool m_resetDelay;
    int m_workingDelay;
    unsigned int m_delayFinderCounter;
    long m_estimatedDelay;
    bool m_error;
    std::atomic<bool>       m_onReset;

    Container::Circular<float> m_data, m_reference, m_loopBuffer;
    struct Meters {
        std::unordered_map<::Abstract::LevelsData::Key, Meter, ::Abstract::LevelsData::Key::Hash> m_meters;
        Meter m_reference;
        std::shared_ptr<math::Filter> m_filter;

        Meters();
        void setSampleRate(unsigned int sampleRate);
        void add(float value);
        void addToReference(const float &value);
        void reset();
    } m_levelMeters;

    FourierTransform m_dataFT;
    Deconvolution m_deconvolution, m_delayFinder;

    Averaging<float> m_deconvAvg;
    Averaging<float> m_magnitudeAvg, m_moduleAvg;
    Averaging<Complex> m_pahseAvg;
    Coherence m_coherence;

    Container::array<Filter::BesselLPF<float>> m_moduleLPFs, m_magnitudeLPFs, m_deconvLPFs;
    Container::array<Filter::BesselLPF<Complex>> m_phaseLPFs;
    Container::array<Meter> m_meters;

    void calculateDataLength();
    void averaging();

    bool m_enableCalibration, m_calibrationLoaded;
    QList<QVector<float>> m_calibrationList;
    QVector<float> m_calibrationGain;
    QVector<float> m_calibrationPhase;
    void applyCalibration();

    std::pair<std::shared_ptr<math::Filter>, std::shared_ptr<math::Filter>> m_inputFilters;

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
    void offsetChanged(float) override;
    void modeChanged(Meta::Measurement::Mode) override;
    void averageChanged(unsigned int) override;
    void dataChanelChanged(unsigned int) override;
    void referenceChanelChanged(unsigned int) override;
    void averageTypeChanged(Meta::Measurement::AverageType) override;
    void windowFunctionTypeChanged(WindowFunction::Type) override;
    void filtersFrequencyChanged(Filter::Frequency) override;
    void delayChanged(int) override;
    void inputFilterChanged(Meta::Measurement::InputFilter) override;
};

#endif // MEASUREMENT_H
