/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#ifndef PHASEPLOT_H
#define PHASEPLOT_H

#include "frequencybasedplot.h"

namespace Chart {
class PhasePlot : public FrequencyBasedPlot
{
    Q_OBJECT

    Q_PROPERTY(int rotate READ rotate WRITE setRotate NOTIFY rotateChanged)
    Q_PROPERTY(int range READ range WRITE setRange NOTIFY rangeChanged)
    Q_PROPERTY(bool positivePeriod READ positivePeriod WRITE setPositivePeriod NOTIFY positivePeriodChanged)

public:
    PhasePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    int rotate() const noexcept;
    void setRotate(int r) noexcept;

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    int range() const noexcept;
    void setRange(int range) noexcept;

    bool positivePeriod() const;
    void setPositivePeriod(bool positivePeriod);

    Q_INVOKABLE void resetAxis() override;
    Q_INVOKABLE void beginGesture() override;
    Q_INVOKABLE void applyGesture(QPointF base, QPointF move, QPointF scale) override;

signals:
    void rotateChanged(int);
    void rangeChanged(int);
    void positivePeriodChanged(bool);

protected:
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;
    bool applyYGesture(qreal base, qreal move, qreal scale);

    int m_center, m_range;
    bool m_positivePeriod;

    struct {
        float min;
        float max;
        float period;
        float offset;
        int center;
        int range;
    } m_storeY;
};
};
#endif // PHASEPLOT_H
