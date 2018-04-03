#ifndef FFTCHART_H
#define FFTCHART_H

#include <array>

#include "src/chartable.h"
#include "painteditem.h"
#include "axis.h"

namespace Fftchart {

enum Type {null, RTA, Magnitude, Phase, Scope, Impulse};

class Chart : public PaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString type READ typeString WRITE setTypeByString)

private:
    Type _type = null;
    Axis *axisX, *axisY;
    std::map<Type, QString> typeMap;

public:
    Chart(QQuickItem *parent = Q_NULLPTR);

    QString typeString() const;
    void setType(const Type type);
    void setTypeByString(const QString &type);

    void paint(QPainter *painter);

    Q_INVOKABLE void appendDataSource(Chartable *source);

public slots:
    void needUpdate();
};
}
#endif // FFTCHART_H
