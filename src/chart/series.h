#ifndef FFTSERIES_H
#define FFTSERIES_H

#include <QtQuick/QQuickPaintedItem>
#include <QColor>
#include "src/chartable.h"
namespace Fftchart {
class Series : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

private:
    QColor m_color;
    Chartable *_source;

public:
    Series(Chartable *source, QQuickItem *parent = Q_NULLPTR);

    QColor color() const { return m_color;}
    void setColor(const QColor &color) {m_color = color;}

    void paint(QPainter *painter);

signals:
    void colorChanged();

public slots:
    void needUpdate();
};
}
#endif // FFTSERIES_H
