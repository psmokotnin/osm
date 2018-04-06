#include "series.h"

#include <cmath>
#include <QPen>
#include <QPainter>

using namespace Fftchart;

Series::Series(Chartable *source, Type *type, Axis *axisX, Axis *axisY, QQuickItem *parent)
    : PaintedItem(parent)
{
    _source = source;
    _type   = type;
    _axisX  = axisX;
    _axisY  = axisY;
    prepareConvert();
}
void Series::prepareConvert()
{
    if (_axisX->type() == AxisType::linear) {
        _xadd = _mm_set_ps1(-1 * _axisX->min());
        _xmul = _mm_set_ps1(width() / (_axisX->max() - _axisX->min()));
    } else {
        _xadd = _mm_set_ps1(-1 * std::log(_axisX->min()));
        _xmul = _mm_set_ps1(width() / std::log(_axisX->max() / _axisX->min()));
    }

    if (_axisY->type() == AxisType::linear) {
        _yadd = _mm_set_ps1(_axisY->max());
        _ymul = _mm_set_ps1(height() / (_axisY->max() - _axisY->min()));
    } else {
        throw std::invalid_argument("_axisY logarithmic scale.");
    }
}
void Series::paint(QPainter *painter)
{
    QPen pen(_source->color(), 2);
    painter->setPen(pen);
    painter->setRenderHints(QPainter::Antialiasing, true);

    //TODO: call if only width or height are changed!
    prepareConvert();

    QPainterPath path;
    int c = _source->dataLength(_type);
    unsigned int lastx = -1;

    for (int i = 1; i < c; i += 4) {

        x[0] = _source->x(_type, i + 0);
        x[1] = _source->x(_type, i + 1);
        x[2] = _source->x(_type, i + 2);
        x[3] = _source->x(_type, i + 3);

        y[0] = _source->y(_type, i + 0);
        y[1] = _source->y(_type, i + 1);
        y[2] = _source->y(_type, i + 2);
        y[3] = _source->y(_type, i + 3);

        paint4(&path, &x, &y, &lastx);

    }
    painter->drawPath(path);
}
void Series::paint4(QPainterPath *path, v4sf *x, v4sf *y, unsigned int *lastx)
{
    const static v4sf SIGNMASK = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
    *x = log_ps(*x);
    *x = _mm_add_ps(*x, _xadd);
    *x = _mm_mul_ps(*x, _xmul);

    *y = _mm_xor_ps(*y, SIGNMASK);    // y = - y
    *y = _mm_add_ps(*y, _yadd);       // y += max
    *y = _mm_mul_ps(*y, _ymul);       // y *= height() / (max - min)

    if (path->isEmpty()) {
        path->moveTo((*x)[0], (*y)[0]);
    }

    for (unsigned int j = 0; j < 4; j++) {
        if (*lastx == (unsigned int)(*x)[j])
            continue;
        path->lineTo((*x)[j], (*y)[j]);
        *lastx = (unsigned int)(*x)[j];
    }
}
void Series::needUpdate()
{
    update();
}
