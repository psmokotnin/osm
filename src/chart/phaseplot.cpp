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
#include <QtMath>

using namespace Chart;
static unsigned int PHASE_TICKS = 9;

PhasePlot::PhasePlot(Settings *settings, QQuickItem *parent): FrequencyBasedPlot(settings, parent),
    m_center(0), m_range(360), m_positivePeriod(false)
{
    m_y.configure(AxisType::Linear,
                  static_cast<float>(-M_PI),
                  static_cast<float>(M_PI),
                  PHASE_TICKS, 180.f / static_cast<float>(M_PI)
                 );
    m_y.setPeriodStart(-M_PI);
    m_y.setPeriodic(2 * static_cast<float>(M_PI));
    m_y.setUnit("°");

    setFlag(QQuickItem::ItemHasContents);
}

int PhasePlot::rotate() const noexcept
{
    return m_center;
}

void PhasePlot::setRotate(int r) noexcept
{
    if (m_center == r)
        return;

    m_center = r;
    m_y.setOffset(m_center);
    emit rotateChanged(m_center);
    update();
}
void PhasePlot::setRange(int range) noexcept
{
    if (m_range == range)
        return;

    m_range = range;
    m_y.setMin(-range * M_PI / 360.0);
    m_y.setMax(range * M_PI / 360.0);
    emit rangeChanged(m_range);
    update();
}

void PhasePlot::resetAxis()
{
    setRotate(0);
    setRange(360);
    XYPlot::resetAxis();
}

void PhasePlot::beginGesture()
{
    XYPlot::beginGesture();
    m_storeY.min = m_y.min();
    m_storeY.max = m_y.max();
    m_storeY.offset = m_y.offset();
    m_storeY.period = m_y.period();
    m_storeY.center = m_center;
    m_storeY.range = m_range;
}

void PhasePlot::applyGesture(QPointF base, QPointF move, QPointF scale)
{
    bool tx = m_x.applyGesture(base.x(), move.x(), scale.x());
    bool ty = applyYGesture(base.y(), move.y(), scale.y());
    if (tx || ty) {
        update();
    }
}
bool PhasePlot::applyYGesture(qreal base, qreal move, qreal scale)
{
    auto reverse = [this](float value) {
        float l = value * (m_storeY.max - m_storeY.min) / m_y.pheight() +
                  m_storeY.min + m_storeY.offset / m_y.scale();

        while (std::abs(l) > m_storeY.period / 2.f) {
            l -= std::copysign(m_storeY.period, l);
        }
        return l;
    };

    float start  = reverse(base - m_padding.top);
    float target = reverse(base + move - m_padding.top);
    auto newCenter = m_storeY.center - (target - start) * 180 / M_PI;
    if (newCenter >  180) newCenter -= 360;
    if (newCenter < -180) newCenter += 360;
    setRotate(newCenter);

    if (!qFuzzyCompare(scale, 1)) {
        auto newRange = scale * (m_storeY.range - target) + target;
        setRange(newRange);
    }

    return true;
}

bool PhasePlot::positivePeriod() const
{
    return m_positivePeriod;
}

void PhasePlot::setPositivePeriod(bool positivePeriod)
{
    if (m_positivePeriod != positivePeriod) {
        m_positivePeriod = positivePeriod;
        emit positivePeriodChanged(m_positivePeriod);

        if (m_positivePeriod) {
            m_y.setPeriodStart(0);
            m_y.autoLabels(PHASE_TICKS);
        } else {
            m_y.setPeriodStart(-M_PI);
            m_y.autoLabels(PHASE_TICKS);
        }
        m_y.update();
    }
}
void PhasePlot::setSettings(Settings *settings) noexcept
{
    if (settings && (settings->value("type") == "Phase")) {
        FrequencyBasedPlot::setSettings(settings);
        setRotate(
            m_settings->reactValue<PhasePlot, int>("rotate", this, &PhasePlot::rotateChanged,
                                                   m_center).toInt());
        setRange(
            m_settings->reactValue<PhasePlot, int>("range", this, &PhasePlot::rangeChanged, m_range).toInt());

        setPositivePeriod(m_settings->reactValue<PhasePlot, bool>(
                              "positivePeriod",
                              this,
                              &PhasePlot::positivePeriodChanged,
                              m_positivePeriod).toBool());
    }
}
void PhasePlot::storeSettings() noexcept
{
    if (!m_settings)
        return;

    FrequencyBasedPlot::storeSettings();
    m_settings->setValue("rotate", m_center);
    m_settings->setValue("range", m_range);
    m_settings->setValue("positivePeriod", m_positivePeriod);
}

int PhasePlot::range() const noexcept
{
    return m_range;
}
