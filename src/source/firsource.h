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
#include "math/fouriertransform.h"
#include "shared/source_shared.h"

class FirSource : public Abstract::Source
{
    Q_OBJECT

    Q_PROPERTY(Shared::Source source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QUuid sourceId READ sourceId NOTIFY sourceChanged)
    Q_PROPERTY(int smoothing READ smoothing WRITE setSmoothing NOTIFY smoothingChanged)
    Q_PROPERTY(float highpass READ highpass WRITE setHighpass NOTIFY highpassChanged)
    Q_PROPERTY(float lowpass READ lowpass WRITE setLowpass NOTIFY lowpassChanged)
    Q_PROPERTY(int tapCount READ tapCount WRITE setTapCount NOTIFY tapCountChanged)
    Q_PROPERTY(float slope READ slope WRITE setSlope NOTIFY slopeChanged)
    Q_PROPERTY(float minFrequency READ minFrequency NOTIFY minFrequencyChanged)
    Q_PROPERTY(QVariant smoothingValues READ smoothingValues CONSTANT)

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

    float slope() const;
    void setSlope(float dbPerOctave);

    float minFrequency() const;

    QVariant smoothingValues() const;

public slots:
    void update();

signals:
    void sourceChanged();
    void smoothingChanged();
    void highpassChanged();
    void lowpassChanged();
    void tapCountChanged();
    void slopeChanged();
    void minFrequencyChanged();

private:
    // Interpolate complex H from m_ftdata at the given frequency.
    // m_ftdata must already be populated (after smoothing/filtering).
    Complex interpolateH(float freq) const;

    // Gaussian smoothing of magnitudes in dB/log-frequency space.
    // Kernel sigma = 1/(2*ppo) octaves, so the ±3-sigma window spans ±1.5/ppo oct.
    // Works on linear magnitudes; converts to dB internally for symmetric weighting.
    std::vector<float> applySmoothing(const std::vector<float> &mag, int ppo) const;

    // Apply LP/HP power-law rolloff cutoffs in-place.
    // fMin is the hard lower bound (bins below it are zeroed).
    void applyCutoffs(std::vector<Complex> &H, float fMin) const;

    Shared::Source m_source;
    int   m_smoothing;       // 0=off, 1, 3, 6, 12, 24
    float m_highpass;        // Hz, 0 = auto (uses minFrequency)
    float m_lowpass;         // Hz, 0=off
    int   m_tapCount;        // 512 or 1024
    float m_slope;           // dB/octave rolloff for HP/LP
    float m_minFrequency;    // sr / tapCount, computed in update()

    FourierTransform m_inverse;
};

#endif // FIRSOURCE_H
