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
    x(AxisDirection::horizontal, m_palette, this),
    y(AxisDirection::vertical, m_palette, this)
{
}

qreal XYPlot::x2v(qreal mouseX) const noexcept
{
    return x.coordToValue(mouseX);
}
qreal XYPlot::y2v(qreal mouseY) const noexcept
{
    return y.coordToValue(mouseY);
}
void XYPlot::setXMin(float v)
{
    if (qFuzzyCompare(v, x.min()))
        return;

    x.setMin(v);
    emit xminChanged(x.min());
    update();
}
void XYPlot::setXMax(float v)
{
    if (qFuzzyCompare(v, x.max()))
        return;

    x.setMax(v);
    emit xmaxChanged(x.max());
    update();
}
void XYPlot::setYMin(float v)
{
    if (qFuzzyCompare(v, y.min()))
        return;

    y.setMin(v);
    emit yminChanged(y.min());
    update();
}
void XYPlot::setYMax(float v)
{
    if (qFuzzyCompare(v, y.max()))
        return;

    y.setMax(v);
    emit ymaxChanged(y.max());
    update();
}
void XYPlot::setSettings(Settings *settings) noexcept
{
    Plot::setSettings(settings);
    x.setMin(m_settings->reactValue<XYPlot, float>("xmin", this, &XYPlot::xminChanged, x.min()).toFloat());
    x.setMax(m_settings->reactValue<XYPlot, float>("xmax", this, &XYPlot::xmaxChanged, x.max()).toFloat());
    y.setMin(m_settings->reactValue<XYPlot, float>("ymin", this, &XYPlot::yminChanged, y.min()).toFloat());
    y.setMax(m_settings->reactValue<XYPlot, float>("ymax", this, &XYPlot::ymaxChanged, y.max()).toFloat());
}
void XYPlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    m_settings->setValue("xmin", x.min());
    m_settings->setValue("xmax", x.max());
    m_settings->setValue("ymin", y.min());
    m_settings->setValue("ymax", y.max());

}
