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

    Q_INVOKABLE qreal x2v(qreal x) const noexcept;
    Q_INVOKABLE qreal y2v(qreal y) const noexcept;
    Q_INVOKABLE QString urlForGrab(QUrl url) const {return url.toLocalFile();}

signals:
    void typeChanged();
    void pointsPerOctaveChanged();

public slots:
    void needUpdate();
};
}
#endif // FFTCHART_H
