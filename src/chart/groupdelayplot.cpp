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
#include "groupdelayplot.h"
#include "groupdelayseriesrenderer.h"

using namespace Fftchart;

GroupDelayPlot::GroupDelayPlot(Settings *settings, QQuickItem *parent): XYPlot(settings, parent),
    m_pointsPerOctave(12), m_coherence(true)
{
    x.configure(AxisType::logarithmic, 20.f, 20000.f);
    x.setISOLabels();
    y.configure(AxisType::linear,
                -50.f * static_cast<float>(M_PI) / 500,  //min
                50.f  * static_cast<float>(M_PI) / 500,   //max
                21,     //ticks
                500.f / static_cast<float>(M_PI)    //scale
                );
    y.setMin(-20.f * static_cast<float>(M_PI) / 500);
    y.setMax( 20.f * static_cast<float>(M_PI) / 500);
    setFlag(QQuickItem::ItemHasContents);
}
SeriesFBO* GroupDelayPlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, [](){return new GroupDelaySeriesRenderer();}, this);
}
void GroupDelayPlot::setPointsPerOctave(unsigned int p)
{
    if (m_pointsPerOctave == p)
        return;

    m_pointsPerOctave = p;
    emit pointsPerOctaveChanged(m_pointsPerOctave);
    update();
}
void GroupDelayPlot::setCoherence(bool coherence) noexcept
{
    if (m_coherence == coherence)
        return;

    m_coherence = coherence;
    emit coherenceChanged(m_coherence);
    update();
}
void GroupDelayPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Group Delay")) {
        XYPlot::setSettings(settings);
        setCoherence(
            m_settings->reactValue<GroupDelayPlot, bool>("coherence", this, &GroupDelayPlot::coherenceChanged, m_coherence).toBool());
        setPointsPerOctave(
            m_settings->reactValue<GroupDelayPlot, unsigned int>("pointsPerOctave", this, &GroupDelayPlot::pointsPerOctaveChanged, m_pointsPerOctave).toUInt());
    }
}
void GroupDelayPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
    m_settings->setValue("coherence", m_coherence);
    m_settings->setValue("pointsPerOctave", m_pointsPerOctave);
}
