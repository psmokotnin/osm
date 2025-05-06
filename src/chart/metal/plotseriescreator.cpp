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
#include "../phasedelayplot.h"
#include "../coherenceplot.h"
#include "../impulseplot.h"
#include "../stepplot.h"
#include "../spectrogramplot.h"
#include "../crestfactorplot.h"
#include "../nyquistplot.h"
#include "../levelplot.h"

#include "rtaseriesnode.h"
#include "impulseseriesnode.h"
#include "stepseriesnode.h"
#include "magnitudeseriesnode.h"
#include "phaseseriesnode.h"
#include "groupdelayseriesnode.h"
#include "phasedelayseriesnode.h"
#include "coherenceseriesnode.h"
#include "spectrogramseriesnode.h"
#include "crestfactorseriesnode.h"
#include "nyquistseriesnode.h"
#include "levelseriesnode.h"

namespace Chart {

template<typename T> SeriesItem *itemNode(Plot *plot, const Shared::Source &source)
{
    auto s = new SeriesItem(source, plot, [](auto item) {
        return new T(item);
    });
    return s;
}

SeriesItem *RTAPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<RTASeriesNode>(this, source);
    auto s = new SeriesItem(source, this, [](auto item) {
        return new RTASeriesNode(item);
    });
    return s;
}

SeriesItem *MagnitudePlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<MagnitudeSeriesNode>(this, source);
}

SeriesItem *PhasePlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<PhaseSeriesNode>(this, source);
}

SeriesItem *CoherencePlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<CoherenceSeriesNode>(this, source);
}

SeriesItem *GroupDelayPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<GroupDelaySeriesNode>(this, source);
}

SeriesItem *PhaseDelayPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<PhaseDelaySeriesNode>(this, source);
}

SeriesItem *ImpulsePlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<ImpulseSeriesNode>(this, source);
}

SeriesItem *StepPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<StepSeriesNode>(this, source);
}

SeriesItem *SpectrogramPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<SpectrogramSeriesNode>(this, source);
}

SeriesItem *CrestFactorPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<CrestFactorSeriesNode>(this, source);
}

SeriesItem *NyquistPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<NyquistSeriesNode>(this, source);
}

SeriesItem *LevelPlot::createSeriesFromSource(const Shared::Source &source)
{
    return itemNode<LevelSeriesNode>(this, source);
}

}
