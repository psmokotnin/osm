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
#include "../crestfactorplot.h"
#include "../nyquistplot.h"

#include "rtaseriesnode.h"
#include "impulseseriesnode.h"
#include "stepseriesnode.h"
#include "magnitudeseriesnode.h"
#include "phaseseriesnode.h"
#include "groupdelayseriesnode.h"
#include "coherenceseriesnode.h"
#include "spectrogramseriesnode.h"
#include "crestfactorseriesnode.h"
#include "nyquistseriesnode.h"

namespace chart {

template<typename T> SeriesItem *itemNode(Plot *plot, Source *source)
{
    auto s = new SeriesItem(source, plot, [](auto item) {
        return new T(item);
    });
    return s;
}

SeriesItem *RTAPlot::createSeriesFromSource(Source *source)
{
    return itemNode<RTASeriesNode>(this, source);
    auto s = new SeriesItem(source, this, [](auto item) {
        return new RTASeriesNode(item);
    });
    return s;
}

SeriesItem *MagnitudePlot::createSeriesFromSource(Source *source)
{
    return itemNode<MagnitudeSeriesNode>(this, source);
}

SeriesItem *PhasePlot::createSeriesFromSource(Source *source)
{
    return itemNode<PhaseSeriesNode>(this, source);
}

SeriesItem *CoherencePlot::createSeriesFromSource(Source *source)
{
    return itemNode<CoherenceSeriesNode>(this, source);
}

SeriesItem *GroupDelayPlot::createSeriesFromSource(Source *source)
{
    return itemNode<GroupDelaySeriesNode>(this, source);
}

SeriesItem *ImpulsePlot::createSeriesFromSource(Source *source)
{
    return itemNode<ImpulseSeriesNode>(this, source);
}

SeriesItem *StepPlot::createSeriesFromSource(Source *source)
{
    return itemNode<StepSeriesNode>(this, source);
}

SeriesItem *SpectrogramPlot::createSeriesFromSource(Source *source)
{
    return itemNode<SpectrogramSeriesNode>(this, source);
}

SeriesItem *CrestFactorPlot::createSeriesFromSource(Source *source)
{
    return itemNode<CrestFactorSeriesNode>(this, source);
}

SeriesItem *NyquistPlot::createSeriesFromSource(Source *source)
{
    return itemNode<NyquistSeriesNode>(this, source);
}

}
