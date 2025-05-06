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
#include "../magnitudeplot.h"
#include "../phaseplot.h"
#include "../groupdelayplot.h"
#include "../phasedelayplot.h"
#include "../impulseplot.h"
#include "../stepplot.h"
#include "../spectrogramplot.h"
#include "../crestfactorplot.h"
#include "../nyquistplot.h"
#include "../levelplot.h"

#include "rtaseriesrenderer.h"
#include "magnitudeseriesrenderer.h"
#include "phaseseriesrenderer.h"
#include "groupdelayseriesrenderer.h"
#include "phasedelayseriesrenderer.h"
#include "coherenceseriesrenderer.h"
#include "impulseseriesrenderer.h"
#include "stepseriesrenderer.h"
#include "spectrogramseriesrenderer.h"
#include "crestfactorseriesrenderer.h"
#include "nyquistseriesrenderer.h"
#include "levelseriesrenderer.h"

namespace Chart {

SeriesFBO *RTAPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new RTASeriesRenderer();
    }, this);
}

SeriesFBO *MagnitudePlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new MagnitudeSeriesRenderer();
    }, this);
}

SeriesFBO *PhasePlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new PhaseSeriesRenderer();
    }, this);
}

SeriesFBO *CoherencePlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new CoherenceSeriesRenderer();
    }, this);
}

SeriesFBO *GroupDelayPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new GroupDelaySeriesRenderer();
    }, this);
}

SeriesFBO *PhaseDelayPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new PhaseDelaySeriesRenderer();
    }, this);
}

SeriesFBO *ImpulsePlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new ImpulseSeriesRenderer();
    }, this);
}

SeriesFBO *StepPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new StepSeriesRenderer();
    }, this);
}

SeriesFBO *SpectrogramPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new SpectrogramSeriesRenderer();
    }, this);
}

SeriesItem *CrestFactorPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new CrestFactorSeriesRenderer();
    }, this);
}

SeriesFBO *NyquistPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new NyquistSeriesRenderer();
    }, this);
}

SeriesFBO *LevelPlot::createSeriesFromSource(const Shared::Source &source)
{
    return new SeriesFBO(source, []() {
        return new LevelSeriesRenderer();
    }, this);
}

}
