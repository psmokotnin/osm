#include "series.h"
#include <QPen>
#include <QPainter>

using namespace Fftchart;

Series::Series(Chartable *source, QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    _source = source;
    connect(source, SIGNAL(colorChanged()), this, SLOT(needUpdate()));
}
void Series::paint(QPainter *painter)
{
    QPen pen(_source->color(), 2);
    painter->setPen(pen);
    painter->setRenderHints(QPainter::Antialiasing, true);
    painter->drawPie(boundingRect().adjusted(1, 1, -1, -1), 90 * 16, 290 * 16);

    QPainterPath path;
    path.moveTo(50, 80);
    path.lineTo(20, 30);
    path.cubicTo(80, 0, 50, 50, 80, 80);
    painter->drawPath(path);
}
void Series::needUpdate()
{
    update();
}
