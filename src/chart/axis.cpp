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
using namespace Fftchart;

vector<float> Axis::ISO_LABELS = {
    31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};

QString Axis::unit() const
{
    return m_unit;
}

void Axis::setUnit(const QString &unit)
{
    m_unit = unit;
}

Axis::Axis(AxisDirection d, const Palette &palette, QQuickItem *parent)
    : PaintedItem(parent),
      m_palette(palette),
      m_min(0.f), m_max(1.f), m_scale(1.f),
      m_lowLimit(0.f), m_highLimit(1.f), m_offset(0.f), m_centralLabel(0.f), m_period()
{
    m_direction = d;

    connect(parent, SIGNAL(widthChanged()), this, SLOT(parentWidthChanged()));
    connect(parent, SIGNAL(heightChanged()), this, SLOT(parentHeightChanged()));
    connect(&m_palette, SIGNAL(changed()), this, SLOT(update()));
    setWidth(parent->width());
    setHeight(parent->height());
}
void Axis::parentWidthChanged()
{
    setWidth(parentItem()->width());
}
void Axis::parentHeightChanged()
{
    setHeight(parentItem()->height());
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
    for_each(m_labels.begin(), m_labels.end(), [&](float & l) {

        try {
            float lv = l - m_offset / scale();

            if (m_period && !qFuzzyCompare(m_period.value_or(0.f), 0.f)) {
                while (std::abs(lv) > m_period.value_or(0.f) / 2.f) {
                    lv -= std::copysign(m_period.value_or(0.f), lv);
                }
            }

            t = convert(lv, size);
        } catch (const invalid_argument &e) {
            qDebug() << l << e.what();
            return; //continue
        }

        p1.setX(static_cast<int>(m_direction == Horizontal ? t + padding.left : padding.left));
        p2.setX(static_cast<int>(m_direction == Horizontal ? t + padding.left : widthf() - padding.right));

        p1.setY(static_cast<int>(m_direction == Horizontal ? heightf() - padding.bottom : heightf() -
                                 padding.bottom - t));
        p2.setY(static_cast<int>(m_direction == Horizontal ? padding.top : heightf() - padding.bottom - t));

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
float Axis::reverse(float value, float size) const noexcept
{
    if (m_type == AxisType::Logarithmic) {
        return pow(static_cast<float>(M_E), log(m_min) + value * log(m_max / m_min) / size);
    }
    float l = value * (m_max - m_min) / size + m_min + m_offset / scale();

    if (m_period && !qFuzzyCompare(m_period.value_or(0.f), 0.f)) {
        while (std::abs(l) > m_period.value_or(0.f) / 2.f) {
            l -= std::copysign(m_period.value_or(0.f), l);
        }
    }

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
    if (std::abs(m_min + m_max) < std::max(abs(m_min), std::abs(m_max))) {
        l = 0;
        m_labels.push_back(l);
        ticks --;
        step = 2 * std::max(std::abs(m_min), std::abs(m_max)) / ticks;
        for (unsigned int i = 0; i < ticks / 2; i++) {
            l += step;
            m_labels.push_back(l);
            m_labels.push_back(-1 * l);
        }
    } else {
        l = m_min;
        step = std::abs(m_max - m_min) / ticks;
        for (unsigned int i = 0; i <= ticks; i++) {
            m_labels.push_back(l);
            l += step;
        }
    }
}
void Axis::needUpdate()
{
    update();
}
void Axis::setMin(float v)
{
    m_min = std::max(m_lowLimit,
                     std::min(v, m_max)
                    );
    needUpdate();
}
void Axis::setMax(float v)
{
    m_max = std::min(m_highLimit,
                     std::max(v, m_min)
                    );
    needUpdate();
}
void Axis::setOffset(float offset)
{
    m_offset = offset;
    needUpdate();
}
void Axis::setCentralLabel(float central)
{
    m_centralLabel = central;
    needUpdate();
}
void Axis::setPeriodic(float p)
{
    m_period = p;
}
