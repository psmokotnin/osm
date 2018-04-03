#ifndef AXIS_H
#define AXIS_H

#include "painteditem.h"

namespace Fftchart {

    enum AxisType {linear, logarithmic};
    enum AxisDirection {vertical, horizontal};

class Axis : public PaintedItem
{
    Q_OBJECT
    static std::vector<qreal> ISO_LABELS;

private:
    AxisType _type = linear;
    AxisDirection _direction;
    QColor _lineColor, _textColor;
    qreal _min, _max;
    std::vector<qreal> _labels;

protected:
    qreal convert(qreal value, qreal size);

public:
    Axis(AxisDirection d, QQuickItem *parent = Q_NULLPTR);
    void paint(QPainter *painter) noexcept;

    void configure(AxisType type, qreal min, qreal max, unsigned int ticks = 0);

    void setMin(qreal v) {_min = v;}
    void setMax(qreal v) {_max = v;}

    void setISOLabels() {_labels = ISO_LABELS;}
    void autoLabels(unsigned int ticks);
    //void setLabels(std::vector<qreal> labels) {_labels = labels;}

    void setType(AxisType t) {_type = t;}

public slots:
    void needUpdate();
};
}
#endif // AXIS_H
