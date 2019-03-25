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
#include "phaseplot.h"
#include "phaseseriesrenderer.h"

using namespace Fftchart;

PhasePlot::PhasePlot(QQuickItem *parent): XYPlot(parent),
    m_pointsPerOctave(12), m_coherence(true)
{
    x.configure(AxisType::logarithmic, 20.f, 20000.f);
    x.setISOLabels();
    y.configure(AxisType::linear,
                static_cast<float>(-M_PI),
                static_cast<float>(M_PI),
                9, 180.f / static_cast<float>(M_PI)
                );
    setFlag(QQuickItem::ItemHasContents);
}
SeriesFBO* PhasePlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, [](){return new PhaseSeriesRenderer();}, this);
}
void PhasePlot::setPointsPerOctave(unsigned int p)
{
    if (m_pointsPerOctave == p)
        return;

    m_pointsPerOctave = p;
    emit pointsPerOctaveChanged();
}
void PhasePlot::setCoherence(bool coherence) noexcept
{
    if (m_coherence != coherence) {
        m_coherence = coherence;
        emit coherenceChanged();
    }
}
