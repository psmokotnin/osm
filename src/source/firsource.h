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

    QVariant smoothingValues() const;

public slots:
    void update();

signals:
    void sourceChanged();
    void smoothingChanged();
    void highpassChanged();
    void lowpassChanged();
    void tapCountChanged();

private:
    // Interpolate complex H from m_ftdata at the given frequency.
    // m_ftdata must already be populated (after smoothing/filtering).
    Complex interpolateH(float freq) const;

    // Apply 1/N-octave Gaussian smoothing in the complex domain.
    // smoothingN: octave denominator (1=1oct, 3=1/3oct, ..., 0=off)
    std::vector<Complex> applySmoothing(const std::vector<Complex> &H,
                                        int smoothingN) const;

    // Apply LP/HP cosine-tapered cutoffs in-place.
    void applyCutoffs(std::vector<Complex> &H) const;

    Shared::Source m_source;
    int   m_smoothing;   // 0=off, 1, 3, 6, 12, 24
    float m_highpass;    // Hz, 0=off
    float m_lowpass;     // Hz, 0=off
    int   m_tapCount;    // 512 or 1024

    FourierTransform m_inverse;
};

#endif // FIRSOURCE_H
