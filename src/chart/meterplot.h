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
#ifndef CHART_METERPLOT_H
#define CHART_METERPLOT_H

#include <QtCore>
#include "levelobject.h"
#include "source.h"

namespace chart {

class MeterPlot : public QObject, public LevelObject
{
public:
    Q_OBJECT

    Q_PROPERTY(QVariant availableCurves READ getAvailableCurves CONSTANT)
    Q_PROPERTY(QVariant availableTimes READ getAvailableTimes CONSTANT)

    Q_PROPERTY(chart::Source *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString curve READ curveName WRITE setCurve NOTIFY curveChanged)
    Q_PROPERTY(QString time READ timeName WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(chart::LevelObject::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString modeName READ modeName NOTIFY modeChanged)

    Q_PROPERTY(float value READ value NOTIFY valueChanged)
    Q_PROPERTY(float threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)

public:
    MeterPlot(QObject *parent = nullptr);

    chart::Source *source() const;
    void setSource(chart::Source *source);

    float value() const;

    float threshold() const;
    void setThreshold(float threshold);

signals:
    void curveChanged(QString) override;
    void timeChanged(QString) override;
    void modeChanged(chart::LevelObject::Mode) override;

    void sourceChanged(chart::Source *);
    void valueChanged();
    void thresholdChanged(float);

private slots:
    void updateThreshold();
    void resetSource();

private:
    chart::Source *m_source;
    QMetaObject::Connection m_sourceConnection;
    float m_threshold;
};

} // namespace chart

#endif // CHART_METERPLOT_H
