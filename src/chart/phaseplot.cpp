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

PhasePlot::PhasePlot(Settings *settings, QQuickItem *parent): XYPlot(settings, parent),
    m_pointsPerOctave(12), m_center(0), m_range(360), m_coherence(true)
{
    x.configure(AxisType::logarithmic, 20.f, 20000.f);
    x.setISOLabels();
    y.configure(AxisType::linear,
                static_cast<float>(-M_PI),
                static_cast<float>(M_PI),
                9, 180.f / static_cast<float>(M_PI)
                );
    y.setPeriodic(2 * static_cast<float>(M_PI));
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
    emit pointsPerOctaveChanged(m_pointsPerOctave);
    update();
}

void PhasePlot::setRotate(int r) noexcept
{
    if (m_center == r)
        return;

    m_center = r;
    y.setOffset(m_center);
    emit rotateChanged(m_center);
    update();
}
void PhasePlot::setRange(int range) noexcept
{
    if (m_range == range)
        return;

    m_range = range;
    setYMin(static_cast<float>(-M_PI) * m_range / 360);
    setYMax(static_cast<float>( M_PI) * m_range / 360);
    emit rangeChanged(m_range);
    update();
}
void PhasePlot::setCoherence(bool coherence) noexcept
{
    if (m_coherence == coherence)
        return;

    m_coherence = coherence;
    emit coherenceChanged(m_coherence);
    update();
}
void PhasePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Phase")) {
        XYPlot::setSettings(settings);
        setCoherence(
            m_settings->reactValue<PhasePlot, bool>("coherence", this, &PhasePlot::coherenceChanged, m_coherence).toBool());
        setRotate(
            m_settings->reactValue<PhasePlot, int>("rotate", this, &PhasePlot::rotateChanged, m_center).toInt());
        setRange(
            m_settings->reactValue<PhasePlot, int>("range", this, &PhasePlot::rangeChanged, m_range).toInt());
        setPointsPerOctave(
            m_settings->reactValue<PhasePlot, unsigned int>("pointsPerOctave", this, &PhasePlot::pointsPerOctaveChanged, m_pointsPerOctave).toUInt());
    }
}
void PhasePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
    m_settings->setValue("coherence", m_coherence);
    m_settings->setValue("rotate", m_center);
    m_settings->setValue("range", m_range);
    m_settings->setValue("pointsPerOctave", m_pointsPerOctave);
}
