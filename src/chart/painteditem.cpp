#include <cmath>
#include "painteditem.h"

using namespace Fftchart;

PaintedItem::PaintedItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{

}

QString PaintedItem::format(qreal v)
{
    bool addK = false;
    if (v >= 1000) {
        v /= 1000;
        addK = true;
    }
    v = std::round(v * 10) / 10;
    return QString::number(v) + (addK ? "K" : "");
}
