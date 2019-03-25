/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#include "rtaplot.h"
#include "rtaseriesrenderer.h"

using namespace Fftchart;

RTAPlot::RTAPlot(QQuickItem *parent): XYPlot(parent),
    m_mode(0),
    m_pointsPerOctave(12),
    m_spline(false), m_coherence(false)
{
    x.configure(AxisType::logarithmic, 20.f, 20000.f);
    x.setISOLabels();
    y.configure(AxisType::linear, -90.f,    0.f,  9);
    setFlag(QQuickItem::ItemHasContents);
    connect(this, SIGNAL(modeChanged()), this, SLOT(update()));
    connect(this, SIGNAL(pointsPerOctaveChanged()), this, SLOT(update()));
}
SeriesFBO* RTAPlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, [](){return new RTASeriesRenderer();}, this);
}
void RTAPlot::setPointsPerOctave(unsigned int p)
{
    if (m_pointsPerOctave == p)
        return;

    m_pointsPerOctave = p;
    emit pointsPerOctaveChanged();
}
void RTAPlot::setCoherence(bool coherence) noexcept
{
    if (m_coherence != coherence) {
        m_coherence = coherence;
        emit coherenceChanged();
    }
}
