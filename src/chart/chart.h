#ifndef FFTCHART_H
#define FFTCHART_H

#include <array>
#include "type.h"
#include "axis.h"
#include "source.h"
#include "painteditem.h"

namespace Fftchart {

class Chart : public PaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString type READ typeString WRITE setTypeByString)
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)

private:
    Type _type;//NOTE: std::optional? llvm doesnot support:(
    void _setType(const Type type);
    Axis *axisX, *axisY;
    std::map<Type, QString> typeMap;
    unsigned int _pointsPerOctave = 0;

public:
    Chart(QQuickItem *parent = Q_NULLPTR);

    QString typeString() const;
    void setType(const Type type);
    void setTypeByString(const QString &type);

    void paint(QPainter *painter);

    Q_INVOKABLE void appendDataSource(Source *source);
    Q_INVOKABLE void removeDataSource(Source *source);
    unsigned int pointsPerOctave() {return _pointsPerOctave;}
    void setPointsPerOctave(unsigned int p);

signals:
    void typeChanged();
    void pointsPerOctaveChanged();

public slots:
    void needUpdate();
};
}
#endif // FFTCHART_H
