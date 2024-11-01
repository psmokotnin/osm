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
#include <cmath>
#include <stdexcept>
#include <QPen>
#include <QPainter>
#include <QtMath>
#include "axis.h"

using namespace std;
using namespace Chart;

vector<float> Axis::ISO_LABELS = {
    31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};
Axis::Axis(AxisDirection d, const Palette &palette, QQuickItem *parent)
    : PaintedItem(parent),
      m_direction(d),
      m_palette(palette),
      m_labels(),
      m_min(0.f), m_max(1.f), m_scale(1.f),
      m_lowLimit(0.f), m_highLimit(1.f),
      m_offset(0.f), m_centralLabel(0.f),
      m_helperValue(NAN), m_period(), m_periodStart(0),
      m_unit()
{
    connect(parent, SIGNAL(widthChanged()), this, SLOT(parentWidthChanged()));
    connect(parent, SIGNAL(heightChanged()), this, SLOT(parentHeightChanged()));
    connect(&m_palette, SIGNAL(changed()), this, SLOT(update()));
    setWidth(parent->width());
    setHeight(parent->height());
    setZ(2);
}
void Axis::parentWidthChanged()
{
    setWidth(parentItem()->width());
}
void Axis::parentHeightChanged()
{
    setHeight(parentItem()->height());
}

float Axis::fromPeriodicValue(const float &value) const noexcept
{
    auto v = value;
    auto period = m_period.value_or(0.f);
    if (m_period && !qFuzzyCompare(period, 0.f)) {
        while (v < m_periodStart) {
            v += period;
        }
        while (v > m_periodStart + period) {
            v -= period;
        }
    }

    return v;
}

void Axis::configure(AxisType type, float min, float max, unsigned int ticks, float scale)
{
    setType(type);
    m_lowLimit = min;
    m_highLimit = max;
    setMin(min);
    setMax(max);
    setScale(scale);
    if (type == Linear) {
        autoLabels(ticks);
    }
    m_reset = {
        m_min,
        m_max
    };
    needUpdate();
}
QString Axis::unit() const
{
    return m_unit;
}

void Axis::setUnit(const QString &unit)
{
    if (m_unit != unit) {
        m_unit = unit;
        emit unitChanged(m_unit);
    }
}

float Axis::helperValue() const
{
    return m_helperValue;
}

void Axis::setHelperValue(float helperValue)
{
    m_helperValue = helperValue;
    needUpdate();
}

void Axis::reset()
{
    setMin(m_reset.min);
    setMax(m_reset.max);
}

void Axis::setReset(float min, float max)
{
    m_reset.min = min;
    m_reset.max = max;
}

void Axis::beginGesture()
{
    m_store.min = m_min;
    m_store.max = m_max;
}

bool Axis::applyGesture(qreal base, qreal move, qreal scale)
{
    float size = (m_direction == Horizontal ? pwidth() : pheight());
    float start = reverse(base, size, m_store.min, m_store.max);
    float target = reverse(base + move, size, m_store.min, m_store.max);

    auto applyScale = [&scale, &target, this] (float value) {
        if (m_type == AxisType::Logarithmic) {
            return std::pow(value, scale) * target / std::pow(target, scale);
        }
        return scale * (value - target) + target;
    };
    float newMin, newMax;
    if (m_type == AxisType::Logarithmic) {
        float delta = target / start;
        newMin = applyScale(m_store.min) * delta;
        newMax = applyScale(m_store.max) * delta;
    } else {
        float delta = target - start;
        newMin = applyScale(m_store.min) + delta;
        newMax = applyScale(m_store.max) + delta;
    }

    if (newMin >= m_lowLimit && newMax <= m_highLimit) {
        setMin(newMin);
        setMax(newMax);
        return true;
    }
    return false;
}

float Axis::lowLimit() const noexcept
{
    return m_lowLimit;
}

float Axis::highLimit() const noexcept
{
    return m_highLimit;
}

void Axis::paint(QPainter *painter) noexcept
{
    QPen linePen(m_palette.lineColor(), 1);
    QPen centerLinePen(m_palette.centerLineColor(), 1);
    QPen textPen(m_palette.textColor(), 2);

    painter->setRenderHints(QPainter::Antialiasing, true);

    QPoint p1, p2;
    QRect textRect(0, 0, 50, 20), lastTextRect;
    float t;
    float size = (m_direction == Horizontal ? pwidth() : pheight());
    int alignFlag = static_cast<int>(m_direction == Horizontal ?
                                     Qt::AlignTop | Qt::AlignCenter :
                                     Qt::AlignRight | Qt::AlignHCenter
                                    );

    QRect limit(
        static_cast<int>(padding.left),
        static_cast<int>(padding.top),
        static_cast<int>(widthf()  - padding.left - padding.right ) + 1,
        static_cast<int>(heightf() - padding.top  - padding.bottom) + 1
    );
    auto setPoints = [&](float value) {
        if (value != value) {//isnan
            return;
        }

        try {
            float lv = value - m_offset / scale();

            if (m_period && !qFuzzyCompare(m_period.value_or(0.f), 0.f)) {
                while (lv < m_min) {
                    lv += m_period.value_or(0.f);
                }
                while (lv > m_max) {
                    lv -= m_period.value_or(0.f);
                }
            }

            t = convert(lv, size);
        } catch (const invalid_argument &e) {
            qDebug() << value << e.what();
            return; //continue
        }

        p1.setX(static_cast<int>(m_direction == Horizontal ? t + padding.left : padding.left));
        p2.setX(static_cast<int>(m_direction == Horizontal ? t + padding.left : widthf() - padding.right));

        p1.setY(static_cast<int>(m_direction == Horizontal ? heightf() - padding.bottom : heightf() -
                                 padding.bottom - t));
        p2.setY(static_cast<int>(m_direction == Horizontal ? padding.top : heightf() - padding.bottom - t));
    };

    for_each(m_labels.begin(), m_labels.end(), [&](float & l) {
        setPoints(l);
        //do not draw lines out of padding
        if (!limit.contains(p1) || !limit.contains(p2) )
            return;

        if (qFuzzyCompare(m_centralLabel, l)) {
            painter->setPen(centerLinePen);
        } else {
            painter->setPen(linePen);
        }
        painter->drawLine(p1, p2);

        painter->setPen(textPen);
        textRect.moveTo(
            p1.x() - (m_direction == Horizontal ? textRect.width() / 2 : textRect.width() + 5),
            p1.y() - (m_direction == Horizontal ? 0 : textRect.height() / 2)
        );

        //don't draw next label if it intersects lastone
        if (lastTextRect.isEmpty() || !lastTextRect.intersects(textRect)) {
            painter->drawText(textRect, alignFlag, format(l * scale()));
            lastTextRect = textRect;
        }
    });

    if (!std::isnan(m_helperValue)) {
        QPen cursorPen(m_palette.cursorLineColor(), 1);
        painter->setPen(cursorPen);
        setPoints(m_helperValue / m_scale);
        painter->drawLine(p1, p2);
    }
}
float Axis::convert(float value, float size) const
{
    if (m_type == AxisType::Logarithmic) {
        if (value == 0.f) {
            throw invalid_argument("Value can't be zero at logarithmic scale.");
        }
        return size * (log(value) - log(m_min)) / log(m_max / m_min);
    }
    return size * (value - m_min) / (m_max - m_min);
}
float Axis::reverse(float value, float size, float max, float min) const noexcept
{
    if (isnan(max) || isnan(min)) {
        max = m_max;
        min = m_min;
    }
    if (m_type == AxisType::Logarithmic) {
        return pow(static_cast<float>(M_E), log(min) + value * log(max / min) / size);
    }
    float l = value * (max - min) / size + min + m_offset / scale();

    l = fromPeriodicValue(l);
    return l;
}
float Axis::coordToValue(float coord) const noexcept
{
    float size = (m_direction == Horizontal ? pwidth() : pheight());
    coord = (m_direction == Horizontal ?
             coord - padding.left :
             static_cast<float>(height()) - coord - padding.bottom);
    return reverse(coord, size) * scale();
}
qreal Axis::coordToValue(qreal coord) const noexcept
{
    return static_cast<qreal>(coordToValue(static_cast<float>(coord)));
}
void Axis::autoLabels(unsigned int ticks)
{
    m_labels.clear();
    float l, step;
    //make symetrical labels if _min and _max have different signs
    if (std::abs(m_lowLimit + m_highLimit) < std::max(abs(m_lowLimit), std::abs(m_highLimit))) {
        l = 0;
        m_labels.push_back(l);
        ticks --;
        step = 2 * std::max(std::abs(m_lowLimit), std::abs(m_highLimit)) / ticks;
        for (unsigned int i = 0; i < ticks / 2; i++) {
            l += step;
            m_labels.push_back(fromPeriodicValue(l));
            m_labels.push_back(fromPeriodicValue(-1 * l));
        }
    } else {
        l = m_lowLimit;
        step = std::abs(m_highLimit - m_lowLimit) / ticks;
        for (unsigned int i = 0; i <= ticks; i++) {
            m_labels.push_back(l);
            l += step;
        }
    }
    std::sort(m_labels.begin(), m_labels.end());
}

void Axis::setLabels(std::vector<float> labels) noexcept
{
    m_labels = labels;
    needUpdate();
}

void Axis::setType(AxisType t) noexcept
{
    m_type = t;
    needUpdate();
}

AxisType Axis::type() const noexcept
{
    return m_type;
}

float Axis::offset() const noexcept
{
    return m_offset;
}
void Axis::needUpdate()
{
    update();
}
void Axis::setMin(float v)
{
    auto newValue = std::max(m_lowLimit,
                             std::min(v, m_max)
                            );
    if (!qFuzzyCompare(newValue, m_min)) {
        m_min = newValue;
        needUpdate();
        emit minChanged(m_min);
    }
}

float Axis::min() const noexcept
{
    return m_min;
}

void Axis::setScale(float v) noexcept
{
    m_scale = v;
    needUpdate();
}

float Axis::scale() const noexcept
{
    return m_scale;
}
void Axis::setMax(float v) noexcept
{
    auto newValue = std::min(m_highLimit,
                             std::max(v, m_min)
                            );
    if (!qFuzzyCompare(newValue, m_max)) {
        m_max = newValue;
        needUpdate();
        emit maxChanged(m_max);
    }
}

float Axis::max() const noexcept
{
    return m_max;
}

void Axis::setISOLabels() noexcept
{
    m_labels = ISO_LABELS;
}
void Axis::setOffset(float offset) noexcept
{
    m_offset = offset;
    needUpdate();
}

float Axis::centralLabel() const noexcept
{
    return m_centralLabel;
}
void Axis::setCentralLabel(float central) noexcept
{
    m_centralLabel = central;
    needUpdate();
}
void Axis::setPeriodic(float p)
{
    m_period = p;
    m_periodStart = m_min;
}

float Axis::period() const noexcept
{
    return m_period.value_or(0);
}

void Axis::setPeriodStart(float start)
{
    m_periodStart = start;
}

float Axis::periodStart() const noexcept
{
    return m_periodStart;
}
