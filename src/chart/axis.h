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
#ifndef AXIS_H
#define AXIS_H

#include "painteditem.h"

namespace Fftchart {

    enum AxisType {linear, logarithmic};
    enum AxisDirection {vertical, horizontal};

class Axis : public PaintedItem
{
    Q_OBJECT
    static std::vector<float> ISO_LABELS;

private:
    AxisType _type = linear;
    AxisDirection _direction;
    QColor _lineColor, _textColor;
    float _min, _max, _scale;
    std::vector<float> _labels;

public:
    Axis(AxisDirection d, QQuickItem *parent = Q_NULLPTR);
    void paint(QPainter *painter) noexcept;
    float convert(float value, float size) const;
    float reverse(float value, float size) const noexcept;
    float coordToValue(float coord) const noexcept;
    qreal coordToValue(qreal coord) const noexcept;

    void configure(AxisType type, float min, float max, unsigned int ticks = 0, float scale = 1.0);

    void setMin(float v) {_min = v;}
    float min() const {return _min;}

    void setScale(float v) {_scale = v;}
    float scale() const {return _scale;}

    void setMax(float v) {_max = v;}
    float max() const {return _max;}

    void setISOLabels() {_labels = ISO_LABELS;}
    void autoLabels(unsigned int ticks);

    void setType(AxisType t) {_type = t;}
    AxisType type() const {return _type;}

public slots:
    void needUpdate();
};
}
#endif // AXIS_H
