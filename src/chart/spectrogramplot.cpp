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
#include "spectrogramplot.h"
#include "spectrogramseriesrenderer.h"

using namespace Fftchart;

SpectrogramPlot::SpectrogramPlot(Settings *settings,
                                 QQuickItem *parent): FrequencyBasedPlot(settings, parent)
{
    m_x.configure(AxisType::Logarithmic, 20.f, 20000.f);
    m_x.setISOLabels();
    m_y.configure(AxisType::Linear, 0.f,    4.f,  4);
    setPointsPerOctave(48);
    setFlag(QQuickItem::ItemHasContents);
    m_y.setCentralLabel(m_y.min() - 1.f);
    connect(this, SIGNAL(pointsPerOctaveChanged(unsigned int)), this, SLOT(update()));
}
SeriesFBO *SpectrogramPlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, []() {
        return new SpectrogramSeriesRenderer();
    }, this);
}
