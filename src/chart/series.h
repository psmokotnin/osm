#ifndef FFTSERIES_H
#define FFTSERIES_H

#include <QColor>
#include "../ssemath.h"

#include "axis.h"
#include "type.h"
#include "source.h"

namespace Fftchart {
class Series : public PaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

private:
    QColor m_color;
    Source *_source;
    Axis *_axisX, *_axisY;
    qreal lWidth, lHeight;
    Type *_type;
    v4sf x, _xmul, _xadd, y, _yadd, _ymul;
    unsigned int _pointsPerOctave = 0;
    float _frequencyFactor;

    void convert4Vertexes(v4sf *x, v4sf *y) const;
    void line4Vertexes(v4sf *x, v4sf *y, unsigned int count = 4) const;
    void draw4Bands(v4sf *x, v4sf *y, float *lastX, float width, unsigned int count = 4) const noexcept;

    void bandBars();
    void paintLine(unsigned int size, float (Source::*xFunc)(unsigned int) const, float (Source::*yFunc)(unsigned int) const);
    void smoothLine(float (Source::*valueFunc)(unsigned int) const);

    unsigned int pointsPerOctave() {return _pointsPerOctave;}

public:
    Series(Source *source, Type *type, Axis *axisX, Axis *axisY, QQuickItem *parent = Q_NULLPTR);

    QColor color() const { return m_color;}
    void setColor(const QColor &color) {m_color = color;}
    void paint(QPainter *painter);
    void setPointsPerOctave(unsigned int p);

signals:
    void colorChanged();

public slots:
    void needUpdate();
    void prepareConvert();
};
}
#endif // FFTSERIES_H
