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

#include <QPen>
#include <QPainter>

#include "axis.h"

using namespace std;
using namespace Fftchart;

vector<float> Axis::ISO_LABELS = {
    31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};

Axis::Axis(AxisDirection d, const Palette &palette, QQuickItem *parent)
    : PaintedItem(parent),
      m_palette(palette),
      _min(0.f), _max(1.f), _scale(1.f),
      _lowLimit(0.f), _highLimit(1.f), m_offset(0.f), m_centralLabel(0.f), m_period()
{
    _direction = d;

    connect(parent, SIGNAL(widthChanged()), this, SLOT(parentWidthChanged()));
    connect(parent, SIGNAL(heightChanged()), this, SLOT(parentHeightChanged()));
    connect(&m_palette, SIGNAL(changed()), this, SLOT(update()));
    setWidth(parent->width());
    setHeight(parent->height());
}
void Axis::parentWidthChanged() {
    setWidth(parentItem()->width());
}
void Axis::parentHeightChanged() {
    setHeight(parentItem()->height());
}
void Axis::configure(AxisType type, float min, float max, unsigned int ticks, float scale)
{
    setType(type);
    _lowLimit = min;
    _highLimit = max;
    setMin(min);
    setMax(max);
    setScale(scale);
    if (type == linear) {
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
    float size = (_direction == horizontal ? pwidth() : pheight());
    int alignFlag = static_cast<int>(_direction == horizontal ?
                         Qt::AlignTop | Qt::AlignCenter :
                         Qt::AlignRight | Qt::AlignHCenter
                         );

    QRect limit(
                static_cast<int>(padding.left),
                static_cast<int>(padding.top),
                static_cast<int>(widthf()  - padding.left - padding.right ) + 1,
                static_cast<int>(heightf() - padding.top  - padding.bottom) + 1
                );
    for_each(_labels.begin(), _labels.end(), [&](float &l) {

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

        p1.setX(static_cast<int>(_direction == horizontal ? t + padding.left : padding.left));
        p2.setX(static_cast<int>(_direction == horizontal ? t + padding.left : widthf() - padding.right));

        p1.setY(static_cast<int>(_direction == horizontal ? heightf() - padding.bottom : heightf() - padding.bottom - t));
        p2.setY(static_cast<int>(_direction == horizontal ? padding.top : heightf() - padding.bottom - t));

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
            p1.x() - (_direction == horizontal ? textRect.width() / 2 : textRect.width() + 5),
            p1.y() - (_direction == horizontal ? 0 : textRect.height() / 2)
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
    if (_type == AxisType::logarithmic) {
        if (value == 0.f) {
            throw invalid_argument("Value can't be zero at logarithmic scale.");
        }
        return size * (log(value) - log(_min)) / log(_max / _min);
    }
    return size * (value - _min) / (_max - _min);
}
float Axis::reverse(float value, float size) const noexcept
{
    if (_type == AxisType::logarithmic) {
        return pow(static_cast<float>(M_E), log(_min) + value * log(_max / _min) / size);
    }
    float l = value * (_max - _min) / size + _min + m_offset / scale();

    if (m_period && !qFuzzyCompare(m_period.value_or(0.f), 0.f)) {
        while (std::abs(l) > m_period.value_or(0.f) / 2.f) {
            l -= std::copysign(m_period.value_or(0.f), l);
        }
    }

    return l;
}
float Axis::coordToValue(float coord) const noexcept
{
    float size = (_direction == horizontal ? pwidth() : pheight());
    coord = (_direction == horizontal ?
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
    _labels.clear();
    float l, step;
    //make symetrical labels if _min and _max have different signs
    if (std::abs(_min + _max) < std::max(abs(_min), std::abs(_max))) {
        l = 0;
        _labels.push_back(l);
        ticks --;
        step = 2 * std::max(std::abs(_min), std::abs(_max)) / ticks;
        for (unsigned int i = 0; i < ticks / 2; i++) {
            l += step;
            _labels.push_back(l);
            _labels.push_back(-1 * l);
        }
    } else {
        l = _min;
        step = std::abs(_max - _min) / ticks;
        for (unsigned int i = 0; i <= ticks; i++) {
            _labels.push_back(l);
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
    _min = std::max(_lowLimit,
                    std::min(v, _max)
        );
    needUpdate();
}
void Axis::setMax(float v)
{
    _max = std::min(_highLimit,
                   std::max(v, _min)
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
