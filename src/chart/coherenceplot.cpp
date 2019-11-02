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
#include "coherenceplot.h"
#include "coherenceseriesrenderer.h"

using namespace Fftchart;

CoherencePlot::CoherencePlot(Settings *settings, QQuickItem *parent): XYPlot(settings, parent),
    m_pointsPerOctave(12), m_type(Type::NORMAL)
{
    x.configure(AxisType::logarithmic, 20.f, 20000.f);
    x.setISOLabels();
    std::vector<float> labels {0.f, 0.2f, 0.4f, 0.6f, 0.8f, 1.f};
    y.configure(AxisType::linear, 0.f, 1.f);
    y.setLabels(labels);
    setFlag(QQuickItem::ItemHasContents);
}
SeriesFBO* CoherencePlot::createSeriesFromSource(Source *source)
{
    return new SeriesFBO(source, [](){return new CoherenceSeriesRenderer();}, this);
}
void CoherencePlot::setPointsPerOctave(unsigned int p)
{
    if (m_pointsPerOctave == p)
        return;

    m_pointsPerOctave = p;
    emit pointsPerOctaveChanged(m_pointsPerOctave);
    update();
}

void CoherencePlot::setType(CoherencePlot::Type type)
{
    if (m_type != type) {
        m_type = type;
        emit typeChanged(m_type);
        update();
    }
}
void CoherencePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Coherence")) {
        XYPlot::setSettings(settings);

//BUG: settings file broken
//        setType(
//            m_settings->reactValue<CoherencePlot, CoherencePlot::Type>("type", this, &CoherencePlot::typeChanged, m_type));
        setPointsPerOctave(
            m_settings->reactValue<CoherencePlot, unsigned int>("pointsPerOctave", this, &CoherencePlot::pointsPerOctaveChanged, m_pointsPerOctave).toUInt());
    }
}
void CoherencePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
//    m_settings->setValue("type", m_type);
    m_settings->setValue("pointsPerOctave", m_pointsPerOctave);
}
