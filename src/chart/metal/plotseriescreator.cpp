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
#include "../rtaplot.h"
#include "../magnitudeplot.h"
#include "../phaseplot.h"
#include "../groupdelayplot.h"
#include "../coherenceplot.h"
#include "../impulseplot.h"
#include "../stepplot.h"
#include "../spectrogramplot.h"

namespace chart {

SeriesItem *RTAPlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

SeriesItem *MagnitudePlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

SeriesItem *PhasePlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

SeriesItem *CoherencePlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

SeriesItem *GroupDelayPlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

SeriesItem *ImpulsePlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

SeriesItem *StepPlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

SeriesItem *SpectrogramPlot::createSeriesFromSource(Source *source)
{
    return nullptr;
}

}
