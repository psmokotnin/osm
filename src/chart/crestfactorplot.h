/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#ifndef CHART_CRESTFACTORPLOT_H
#define CHART_CRESTFACTORPLOT_H

#include "frequencybasedplot.h"

namespace Chart {

class CrestFactorPlot : public Chart::FrequencyBasedPlot
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY
               pointsPerOctaveChanged)

public:
    CrestFactorPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    unsigned int pointsPerOctave() const;
    void setPointsPerOctave(unsigned int p);

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

signals:
    void pointsPerOctaveChanged(unsigned int);

protected:
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;
};

} // namespace chart

#endif // CHART_CRESTFACTORPLOT_H
