#ifndef FFTCHART_H
#define FFTCHART_H

#include <array>

#include "type.h"
#include "axis.h"
#include "painteditem.h"

#include "src/chartable.h"

namespace Fftchart {

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

signals:
    void typeChanged();

public slots:
    void needUpdate();
};
}
#endif // FFTCHART_H
