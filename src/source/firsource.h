/**
 *  OSM
 *  Copyright (C) 2024  Pavel Smokotnin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FIRSOURCE_H
#define FIRSOURCE_H

#include <QObject>
#include "abstract/source.h"
#include "shared/source_shared.h"

//! Correction FIR filter designed from another source.
//! The frequency and impulse data show what the exported taps actually do.
class FirSource : public Abstract::Source
{
    Q_OBJECT

    Q_PROPERTY(Shared::Source source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QUuid sourceId READ sourceId NOTIFY sourceChanged)
    Q_PROPERTY(int smoothing READ smoothing WRITE setSmoothing NOTIFY smoothingChanged)
    Q_PROPERTY(float highpass READ highpass WRITE setHighpass NOTIFY highpassChanged)
    Q_PROPERTY(float lowpass READ lowpass WRITE setLowpass NOTIFY lowpassChanged)
    Q_PROPERTY(int tapCount READ tapCount WRITE setTapCount NOTIFY tapCountChanged)
    Q_PROPERTY(int transition READ transition WRITE setTransition NOTIFY transitionChanged)
    Q_PROPERTY(bool highpassEnabled READ highpassEnabled WRITE setHighpassEnabled NOTIFY highpassEnabledChanged)
    Q_PROPERTY(bool lowpassEnabled READ lowpassEnabled WRITE setLowpassEnabled NOTIFY lowpassEnabledChanged)
    Q_PROPERTY(int targetSampleRate READ targetSampleRate WRITE setTargetSampleRate NOTIFY targetSampleRateChanged)
    Q_PROPERTY(int phaseMode READ phaseMode WRITE setPhaseMode NOTIFY phaseModeChanged)
    Q_PROPERTY(float maxBoost READ maxBoost WRITE setMaxBoost NOTIFY maxBoostChanged)
    Q_PROPERTY(float minFrequency READ minFrequency NOTIFY minFrequencyChanged)
    Q_PROPERTY(float latency READ latency NOTIFY latencyChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QVariant smoothingValues READ smoothingValues CONSTANT)
    Q_PROPERTY(QVariant sampleRateValues READ sampleRateValues CONSTANT)

public:
    explicit FirSource(QObject *parent = nullptr);

    Shared::Source clone() const override;
    Q_INVOKABLE QJsonObject toJSON() const noexcept override;
    void fromJSON(QJsonObject data, const SourceList *list = nullptr) noexcept override;

    Q_INVOKABLE bool exportFIR(const QUrl &fileName) const noexcept;

    Shared::Source source() const;
    void setSource(const Shared::Source &source);
    QUuid sourceId() const;

    int smoothing() const;
    void setSmoothing(int smoothing);

    float highpass() const;
    void setHighpass(float hz);

    float lowpass() const;
    void setLowpass(float hz);

    int tapCount() const;
    void setTapCount(int taps);

    int transition() const;
    void setTransition(int exponent);

    bool highpassEnabled() const;
    void setHighpassEnabled(bool enabled);

    bool lowpassEnabled() const;
    void setLowpassEnabled(bool enabled);

    int targetSampleRate() const;
    void setTargetSampleRate(int hz);

    int phaseMode() const;
    void setPhaseMode(int mode);

    float maxBoost() const;
    void setMaxBoost(float dB);

    float minFrequency() const;
    float latency() const;
    bool ready() const;

    QVariant sampleRateValues() const;
    QVariant smoothingValues() const;

public slots:
    //! recalculate now
    void update();

    //! recalculate once on the next event loop pass, coalescing repeated requests
    void scheduleUpdate();

signals:
    void sourceChanged();
    void smoothingChanged();
    void highpassChanged();
    void lowpassChanged();
    void tapCountChanged();
    void transitionChanged();
    void highpassEnabledChanged();
    void lowpassEnabledChanged();
    void targetSampleRateChanged();
    void phaseModeChanged();
    void maxBoostChanged();
    void minFrequencyChanged();
    void latencyChanged();
    void readyChanged();

private:
    Shared::Source m_source;
    int   m_smoothing;              // index in smoothingValues: 0=off, 1, 3, 6, 12, 24
    float m_highpass;               // Hz, lower limit of the correction range
    bool  m_highpassEnabled;
    float m_lowpass;                // Hz, upper limit of the correction range
    bool  m_lowpassEnabled;
    int   m_tapCount;
    int   m_transition;             // exponent of the fade outside the range (3=firm … 8=sharp)
    int   m_targetSampleRate;       // 0 = taken from the source
    int   m_phaseMode;              // FirDesign::PhaseMode
    float m_maxBoost;               // dB
    float m_minFrequency;           // sample rate / taps, computed in update()
    float m_latency;                // ms, computed in update()
    bool  m_updateScheduled = false;

    std::vector<float> m_taps;      // guarded by m_dataMutex
};

#endif // FIRSOURCE_H
