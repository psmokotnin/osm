/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

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
#ifndef CHART_STEPPLOT_H
#define CHART_STEPPLOT_H

#include "xyplot.h"

namespace Chart {

class StepPlot : public XYPlot
{
    Q_OBJECT
    Q_PROPERTY(float zero READ zero() WRITE setZero() NOTIFY zeroChanged)

public:
    StepPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);
    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    float zero() const;
    void setZero(float zero);

signals:
    void zeroChanged(float);

protected:
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;

private:
    float m_zero;

};

} // namespace chart

#endif // CHART_STEPPLOT_H
