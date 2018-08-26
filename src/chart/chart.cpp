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
#include "chart.h"
#include "series.h"
#include <QPainter>

using namespace Fftchart;

Chart::Chart(QQuickItem *parent)
    : PaintedItem(parent)
{
    typeMap[RTA]        = "RTA";
    typeMap[Magnitude]  = "Magnitude";
    typeMap[Phase]      = "Phase";
    typeMap[Scope]      = "Scope";
    typeMap[Impulse]    = "Impulse";

    axisX = new Axis(AxisDirection::horizontal, this);
    axisY = new Axis(AxisDirection::vertical, this);

    _setType(RTA);
}
QString Chart::typeString() const
{
    return typeMap.at(_type);
}
void Chart::setTypeByString(const QString &type)
{
    for (auto it = typeMap.begin(); it != typeMap.end(); ++it) {
        if (it->second == type) {
            setType(it->first);
        }
    }
}
void Chart::setType(const Type type)
{
    if (_type != type) {
        _setType(type);
    }
}
void Chart::_setType(const Type type)
{
    _type = type;

    switch (_type) {
        case RTA:         axisY->configure(AxisType::linear, -90,    0,  9);break;
        case Magnitude:   axisY->configure(AxisType::linear, -18,   18, 13);break;
        case Phase:       axisY->configure(AxisType::linear, -M_PI, M_PI,  9, 180 / M_PI);break;
        case Scope:       axisY->configure(AxisType::linear, -1,     1, 11);break;
        case Impulse:     axisY->configure(AxisType::linear, -1,     1, 11);break;
    }

    switch (_type) {
        case RTA:
        case Magnitude:
        case Phase:
            axisX->configure(AxisType::logarithmic, 20, 20000);
            axisX->setISOLabels();
            break;

        case Impulse:
            axisX->configure(AxisType::linear,  -5,  5, 11);break;

        case Scope:
            axisX->configure(AxisType::linear, -10, 10, 11);break;
    }

    emit typeChanged();

    foreach (QQuickItem *item, childItems()) {
        item->update();
    }
    needUpdate();
}
void Chart::paint(QPainter *painter)
{
    //paint axiss & series
    foreach (QQuickItem *item, childItems()) {

        QPainterPath border;
        QPen pen(QColor::fromRgbF(0, 0, 0, 0.1), 1);
        painter->setPen(pen);
        border.moveTo(x()       + padding.left,  y()        + padding.top);
        border.lineTo(x()       + padding.left,  height()   - padding.bottom);
        border.lineTo(width()   - padding.right, height()   - padding.bottom);
        border.lineTo(width()   - padding.right, y()        + padding.top);
        border.lineTo(x()       + padding.left,  y()        + padding.top);
        painter->fillPath(border, QBrush(QColor(Qt::white)));
        painter->drawPath(border);

        QQuickPaintedItem *child = qobject_cast<QQuickPaintedItem*>(item);

        QString className(child->metaObject()->className());
        if (className.compare("Fftchart::Series") == 0) {
            child->setWidth(pwidth());
            child->setHeight(pheight());
            child->setX(x() + padding.left);
            child->setY(y() + padding.top);
        } else {
            child->setWidth(width());
            child->setHeight(height());
            child->setX(x());
            child->setY(y());
        }
    }
}
void Chart::appendDataSource(Source *source)
{
    Series *s = new Series(source, &_type, axisX, axisY, this);
    s->setPointsPerOctave(pointsPerOctave());
    if (source->objectName() == "Measurement") {
        s->setZ(2.0);
    }
    connect(this,   SIGNAL(typeChanged()),  s, SLOT(prepareConvert()));
    connect(source, SIGNAL(colorChanged()), s, SLOT(needUpdate()));
    connect(source, SIGNAL(readyRead()),    s, SLOT(needUpdate()));
    connect(source, SIGNAL(activeChanged()),s, SLOT(needUpdate()));
}
void Chart::removeDataSource(Source *source)
{
    foreach (QQuickItem *item, childItems()) {
        QQuickPaintedItem *child = qobject_cast<QQuickPaintedItem*>(item);
        QString className(child->metaObject()->className());

        if (className.compare("Fftchart::Series") == 0) {
            Series *s = qobject_cast<Series*>(item);

            if (s->getSource() == source) {
                delete s;
                break;
            }
        }
    }
}
void Chart::needUpdate()
{
    update();
}
void Chart::setPointsPerOctave(unsigned int p)
{
    _pointsPerOctave = p;
    foreach(auto *s, findChildren<Fftchart::Series *>()) {
        s->setPointsPerOctave(p);
        s->needUpdate();
    }
}
qreal Chart::x2v(qreal x) const noexcept
{
    return axisX->coordToValue(x);
}
qreal Chart::y2v(qreal y) const noexcept
{
    return axisY->coordToValue(y);
}
