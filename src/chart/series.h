#ifndef FFTSERIES_H
#define FFTSERIES_H

#include <QColor>
#include "../ssemath.h"

#include "axis.h"
#include "src/chartable.h"

namespace Fftchart {
class Series : public PaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

private:
    QColor m_color;
    Chartable *_source;
    Axis *_axisX, *_axisY;
    Type *_type;
    v4sf x, _xmul, _xadd, y, _yadd, _ymul;

    void paint4(QPainterPath *path, v4sf *x, v4sf *y, unsigned int *lastx);


public:
    Series(Chartable *source, Type *type, Axis *axisX, Axis *axisY, QQuickItem *parent = Q_NULLPTR);

    QColor color() const { return m_color;}
    void setColor(const QColor &color) {m_color = color;}
    //void setType(Type type) {_type = type;}
    void paint(QPainter *painter);

signals:
    void colorChanged();

public slots:
    void needUpdate();
    void prepareConvert();
};
}
#endif // FFTSERIES_H
