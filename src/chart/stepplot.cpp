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
#include "stepplot.h"
using namespace Chart;

StepPlot::StepPlot(Settings *settings, QQuickItem *parent) : XYPlot(settings, parent), m_zero(0)
{
    m_x.configure(AxisType::Linear, -500.0, 500.0, 1001);
    m_x.setReset(-5.f, 5.f);
    m_x.reset();
    m_x.setUnit("ms");

    m_y.configure(AxisType::Linear, -2.0, 2.0, 21);
    m_y.setReset(-1.f, 1.f);
    m_y.reset();
    m_y.setUnit("");

    setFlag(QQuickItem::ItemHasContents);
}

float StepPlot::zero() const
{
    return m_zero;
}

void StepPlot::setZero(float zero)
{
    if (!qFuzzyCompare(m_zero, zero)) {
        m_zero = zero;
        emit zeroChanged(m_zero);
        update();
    }
}

void StepPlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Step")) {
        XYPlot::setSettings(settings);
        setZero(m_settings->reactValue<StepPlot, float>("zero", this, &StepPlot::zeroChanged, zero()).toFloat());
    }
}

void StepPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    XYPlot::storeSettings();
    m_settings->setValue("zero", zero());
}
