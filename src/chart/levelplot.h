/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#ifndef CHART_LEVELPLOT_H
#define CHART_LEVELPLOT_H

#include "xyplot.h"
#include "math/weighting.h"
#include "math/meter.h"

namespace chart {

class LevelPlot : public XYPlot
{
public:
    enum Mode {
        dBfs    = 0x00,
        SPL     = 0x01
    };

    static constexpr float SPL_OFFSET = 140;

    Q_OBJECT
    Q_ENUM(Mode);

    Q_PROPERTY(QVariant availableCurves READ getAvailableCurves CONSTANT)
    Q_PROPERTY(QVariant availableTimes READ getAvailableTimes CONSTANT)

    Q_PROPERTY(QString curve READ curveName WRITE setCurve NOTIFY curveChanged)
    Q_PROPERTY(QString time READ timeName WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    LevelPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);
    void setSettings(Settings *settings) noexcept override;
    void storeSettings() noexcept override;

    QVariant getAvailableCurves() const;
    QVariant getAvailableTimes() const;

    Weighting::Curve curve() const;
    QString curveName() const;
    void setCurve(const QString &curve);

    Meter::Time time() const;
    QString timeName() const;
    void setTime(const QString &time);

    Mode mode() const;
    void setMode(const Mode &mode);
    void setMode(const int &mode);

signals:
    void curveChanged(QString);
    void timeChanged(QString);
    void modeChanged(Mode);

protected:
    virtual SeriesItem *createSeriesFromSource(Source *source) override;

private:
    Weighting::Curve m_curve;
    Meter::Time m_time;
    Mode m_mode;

};

} // namespace chart

#endif // CHART_LEVELPLOT_H
