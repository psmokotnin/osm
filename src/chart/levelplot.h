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
#include "levelobject.h"
#include "math/weighting.h"
#include "math/meter.h"

namespace Chart {

class LevelPlot : public XYPlot, public LevelObject
{
public:
    Q_OBJECT

    Q_PROPERTY(QVariant availableCurves READ getAvailableCurves CONSTANT)
    Q_PROPERTY(QVariant availableTimes READ getAvailableTimes CONSTANT)

    Q_PROPERTY(QString curve READ curveName WRITE setCurve NOTIFY curveChanged)
    Q_PROPERTY(QString time READ timeName WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(Chart::LevelObject::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool pause READ pause WRITE setPause NOTIFY pauseChanged)

public:
    LevelPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);
    void setSettings(Settings *settings) noexcept override;
    void storeSettings() noexcept override;

signals:
    void curveChanged(QString) override;
    void timeChanged(QString) override;
    void modeChanged(Chart::LevelObject::Mode) override;
    void pauseChanged(bool) override;

protected:
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;

private slots:
    void updateAxes();
};

} // namespace chart

#endif // CHART_LEVELPLOT_H
