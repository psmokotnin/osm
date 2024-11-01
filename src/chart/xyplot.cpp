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

using namespace Chart;

XYPlot::XYPlot(Settings *settings, QQuickItem *parent) :
    Plot(settings, parent),
    m_x(AxisDirection::Horizontal, m_palette, this),
    m_y(AxisDirection::Vertical, m_palette, this)
{
    connect(&m_x, &Axis::minChanged, this, [this](auto value) {
        emit xminChanged(value);
        update();
    });
    connect(&m_x, &Axis::maxChanged, this, [this](auto value) {
        emit xmaxChanged(value);
        update();
    });
    connect(&m_y, &Axis::minChanged, this, [this](auto value) {
        emit yminChanged(value);
        update();
    });
    connect(&m_y, &Axis::maxChanged, this, [this](auto value) {
        emit ymaxChanged(value);
        update();
    });

    connect(&m_x, &Axis::unitChanged, this, &Plot::xLabelChanged);
    connect(&m_y, &Axis::unitChanged, this, &Plot::yLabelChanged);

    connect(s_cursorHelper, &CursorHelper::valueUpdated, this, [this]() {
        m_x.setHelperValue(s_cursorHelper->value(xLabel(), "x"));
        m_y.setHelperValue(s_cursorHelper->value(yLabel(), "y"));
    });
}

void XYPlot::setHelper(qreal x, qreal y) noexcept
{
    s_cursorHelper->setValue(xLabel(), "x", x2v(x));
    s_cursorHelper->setValue(yLabel(), "y", y2v(y));
}

void XYPlot::unsetHelper() noexcept
{
    s_cursorHelper->unsetValue(xLabel(), "x");
    s_cursorHelper->unsetValue(yLabel(), "y");
}

qreal XYPlot::x2v(qreal mouseX) const noexcept
{
    return m_x.coordToValue(mouseX);
}
qreal XYPlot::y2v(qreal mouseY) const noexcept
{
    return m_y.coordToValue(mouseY);
}

void XYPlot::beginGesture()
{
    m_x.beginGesture();
    m_y.beginGesture();
}

void XYPlot::applyGesture(QPointF base, QPointF move, QPointF scale)
{
    bool tx = m_x.applyGesture(base.x(), move.x(), scale.x());
    bool ty = m_y.applyGesture(base.y(), move.y(), scale.y());
    if (tx || ty) {
        update();
    }
}

void XYPlot::resetAxis()
{
    m_x.reset();
    m_y.reset();
    update();
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
    m_x.setMin(v);
}
void XYPlot::setXMax(float v)
{
    m_x.setMax(v);
}
void XYPlot::setYMin(float v)
{
    m_y.setMin(v);
}
void XYPlot::setYMax(float v)
{
    m_y.setMax(v);
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

void XYPlot::inheritSettings(const Plot *source)
{
    auto xySource = static_cast<const XYPlot *>(source);
    if (!xySource) {
        return;
    }

    if (xLabel() == xySource->xLabel()) {
        setXMin(xySource->xmin());
        setXMax(xySource->xmax());
    }
}
