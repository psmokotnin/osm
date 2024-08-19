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

using namespace chart;

RTAPlot::RTAPlot(Settings *settings, QQuickItem *parent): FrequencyBasedPlot(settings, parent),
    m_mode(chart::RTAPlot::Mode::Line), m_spline(false), m_showPeaks(true)
{
    qRegisterMetaType<chart::RTAPlot::Mode>();
    qRegisterMetaType<chart::RTAPlot::Scale>();

    m_pointsPerOctave = 0;

    m_y.configure(AxisType::Linear, -140.f, 140.f,  15);
    m_y.setCentralLabel(m_y.min() - 1.f);
    m_y.setUnit("dB");
    setFlag(QQuickItem::ItemHasContents);

    connect(this, &RTAPlot::modeChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::showPeaksChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::pointsPerOctaveChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::scaleChanged, this, &RTAPlot::update);
    connect(this, &RTAPlot::scaleChanged, this, &RTAPlot::updateAxis);
}

void RTAPlot::updateAxis()
{
    switch (m_scale) {
    case DBfs:
        m_y.configure(AxisType::Linear, -140.f, 40.f,  15);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setUnit("dB");
        break;

    case SPL:
        m_y.configure(AxisType::Linear, 40.f, 140.f,  10);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setUnit("dB");
        break;

    case Phon:
        m_y.configure(AxisType::Linear, 40.f, 140.f,  10);
        m_y.setCentralLabel(m_y.min() - 1.f);
        m_y.setUnit("ph");
        break;
    }
}

void RTAPlot::setMode(chart::RTAPlot::Mode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    emit modeChanged(m_mode);
}

void RTAPlot::setMode(unsigned int mode)
{
    setMode(static_cast<chart::RTAPlot::Mode>(mode));
}

RTAPlot::Mode RTAPlot::mode()
{
    return m_mode;
}

void RTAPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Spectrum")) {

        FrequencyBasedPlot::setSettings(settings);
        setScale(m_settings->reactValue<RTAPlot, chart::RTAPlot::Scale>("scale", this, &RTAPlot::scaleChanged,
                                                                        m_scale).toUInt());
        setMode(m_settings->reactValue<RTAPlot, chart::RTAPlot::Mode>("mode", this, &RTAPlot::modeChanged, m_mode).toUInt());
        setShowPeaks(m_settings->reactValue<RTAPlot, bool>("showPeaks", this, &RTAPlot::showPeaksChanged, m_mode).toBool());
    }
}
void RTAPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    FrequencyBasedPlot::storeSettings();
    m_settings->setValue("mode", static_cast<int>(m_mode));
    m_settings->setValue("showPeaks", m_showPeaks);
    m_settings->setValue("scale", static_cast<int>(m_scale));
}

bool RTAPlot::showPeaks() const
{
    return m_showPeaks;
}

void RTAPlot::setShowPeaks(bool showPeaks)
{
    if (m_showPeaks != showPeaks) {
        m_showPeaks = showPeaks;
        emit showPeaksChanged(m_showPeaks);
    }
}

bool RTAPlot::isPointsPerOctaveValid(unsigned int &value) const
{
    return value >= 0 && value <= 48;
}

RTAPlot::Scale RTAPlot::scale() const
{
    return m_scale;
}

void RTAPlot::setScale(Scale newScale)
{
    if (m_scale == newScale)
        return;
    m_scale = newScale;
    emit scaleChanged(newScale);
}

void RTAPlot::setScale(unsigned int newScale)
{
    setScale(static_cast<chart::RTAPlot::Scale>(newScale));
}
