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
#include "impulseplot.h"
#include "seriesfbo.h"
#include "impulseseriesrenderer.h"

using namespace Fftchart;

ImpulsePlot::ImpulsePlot(QQuickItem *parent): XYPlot(parent)
{
    x.configure(AxisType::linear, -20.0, 20.0, 41);
    x.setMin(-5.f);
    x.setMax(5.f);
    y.configure(AxisType::linear, -2.0, 2.0, 21);
    y.setMin(-1.f);
    y.setMax(1.f);
    setFlag(QQuickItem::ItemHasContents);
}
SeriesFBO* ImpulsePlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, [](){return new ImpulseSeriesRenderer();}, this);
}
