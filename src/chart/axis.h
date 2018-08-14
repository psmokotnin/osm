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
