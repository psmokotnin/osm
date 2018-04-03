#include <cmath>

#include <QPen>
#include <QPainter>

#include "axis.h"

using namespace std;
using namespace Fftchart;

vector<qreal> Axis::ISO_LABELS = {
    31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};

Axis::Axis(AxisDirection d, QQuickItem *parent)
    : PaintedItem(parent)
{
    _direction = d;
    _lineColor = QColor::fromRgbF(0, 0, 0, 0.1);
    _textColor = QColor::fromRgbF(0, 0, 0, 1);
}
void Axis::configure(AxisType type, qreal min, qreal max, unsigned int ticks)
{
    setType(type);
    setMin(min);
    setMax(max);
    if (type == linear) {
        autoLabels(ticks);
    }
}
void Axis::paint(QPainter *painter) noexcept
{
    QPen linePen(_lineColor, 1);
    QPen textPen(_textColor, 2);

    painter->setRenderHints(QPainter::Antialiasing, true);

    QPoint p1, p2;
    QRect textRect(0, 0, 50, 20), lastTextRect;
    qreal t;
    qreal size = (_direction == horizontal ? pwidth() : pheight());
    int alignFlag = (_direction == horizontal ?
                         Qt::AlignTop | Qt::AlignCenter :
                         Qt::AlignRight | Qt::AlignHCenter
                         );

    for_each(_labels.begin(), _labels.end(), [&](qreal &l) {

        try {
            t = convert(l, size);
        } catch (const invalid_argument e) {
            qDebug() << l << e.what();
            return; //continue
        }

        p1.setX(_direction == horizontal ? t + padding.left : padding.left);
        p2.setX(_direction == horizontal ? t + padding.left : width() - padding.right);

        p1.setY(_direction == horizontal ? height() - padding.bottom : height() - padding.bottom - t);
        p2.setY(_direction == horizontal ? padding.top : height() - padding.bottom - t);

        painter->setPen(linePen);
        painter->drawLine(p1, p2);

        painter->setPen(textPen);
        textRect.moveTo(
            p1.x() - (_direction == horizontal ? textRect.width() / 2 : textRect.width() + 5),
            p1.y() - (_direction == horizontal ? 0 : textRect.height() / 2)
        );

        //don't draw next label if it intersects lastone
        if (lastTextRect.isEmpty() || !lastTextRect.intersects(textRect)) {
            painter->drawText(textRect, alignFlag, format(l));
            lastTextRect = textRect;
        }
    });
}
qreal Axis::convert(qreal value, qreal size)
{
    if (_type == AxisType::logarithmic) {
        if (value == 0) {
            throw invalid_argument("Value can't be zero at logarithmic scale.");
        }
        return size * (log(value) - log(_min)) / log(_max / _min);
    }
    return size * (value - _min) / (_max - _min);
}
void Axis::autoLabels(unsigned int ticks)
{
    _labels.clear();
    qreal l, step;
    //make symetrical labels if _min and _max have different signs
    if (abs(_min + _max) < max(abs(_min), abs(_max))) {
        l = 0;
        _labels.push_back(l);
        ticks --;
        step = 2 * max(abs(_min), abs(_max)) / ticks;
        for (unsigned int i = 0; i < ticks / 2; i++) {
            l += step;
            _labels.push_back(l);
            _labels.push_back(-1 * l);
        }
    } else {
        l = _min;
        step = abs(_max - _min) / ticks;
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
