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
#include "xyplot.h"

using namespace Fftchart;

XYPlot::XYPlot(Settings *settings, QQuickItem *parent) :
    Plot(settings, parent),
    m_x(AxisDirection::Horizontal, m_palette, this),
    m_y(AxisDirection::Vertical, m_palette, this)
{
}

qreal XYPlot::x2v(qreal mouseX) const noexcept
{
    return m_x.coordToValue(mouseX);
}
qreal XYPlot::y2v(qreal mouseY) const noexcept
{
    return m_y.coordToValue(mouseY);
}

QString XYPlot::xLabel() const
{
    return m_x.unit();
}

QString XYPlot::yLabel() const
{
    return m_y.unit();
}
void XYPlot::setXMin(float v)
{
    if (qFuzzyCompare(v, m_x.min()))
        return;

    m_x.setMin(v);
    emit xminChanged(m_x.min());
    update();
}
void XYPlot::setXMax(float v)
{
    if (qFuzzyCompare(v, m_x.max()))
        return;

    m_x.setMax(v);
    emit xmaxChanged(m_x.max());
    update();
}
void XYPlot::setYMin(float v)
{
    if (qFuzzyCompare(v, m_y.min()))
        return;

    m_y.setMin(v);
    emit yminChanged(m_y.min());
    update();
}
void XYPlot::setYMax(float v)
{
    if (qFuzzyCompare(v, m_y.max()))
        return;

    m_y.setMax(v);
    emit ymaxChanged(m_y.max());
    update();
}
void XYPlot::setSettings(Settings *settings) noexcept
{
    Plot::setSettings(settings);
    m_x.setMin(m_settings->reactValue<XYPlot, float>("xmin", this, &XYPlot::xminChanged,
                                                     m_x.min()).toFloat());
    m_x.setMax(m_settings->reactValue<XYPlot, float>("xmax", this, &XYPlot::xmaxChanged,
                                                     m_x.max()).toFloat());
    m_y.setMin(m_settings->reactValue<XYPlot, float>("ymin", this, &XYPlot::yminChanged,
                                                     m_y.min()).toFloat());
    m_y.setMax(m_settings->reactValue<XYPlot, float>("ymax", this, &XYPlot::ymaxChanged,
                                                     m_y.max()).toFloat());
}
void XYPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    m_settings->setValue("xmin", m_x.min());
    m_settings->setValue("xmax", m_x.max());
    m_settings->setValue("ymin", m_y.min());
    m_settings->setValue("ymax", m_y.max());

}
