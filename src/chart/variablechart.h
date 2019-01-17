#ifndef SELECT_H
#define SELECT_H
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
#include <QtQuick/QQuickItem>
#include "type.h"
#include "source.h"
#include "plot.h"

namespace Fftchart {

class VariableChart : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString type READ typeString WRITE setTypeByString NOTIFY typeChanged)
    Q_PROPERTY(QQuickItem *plot READ plot() NOTIFY typeChanged)

private:
    Plot * s_plot;
    Type m_selected;

protected:
    void initType();

public:
    VariableChart(QQuickItem *parent = Q_NULLPTR);
    QString typeString() const;
    void setType(const Type type);
    void setTypeByString(const QString &type);

    Q_INVOKABLE void appendDataSource(Source *source);
    Q_INVOKABLE void removeDataSource(Source *source);
    Plot* plot() {return s_plot;}

    Q_INVOKABLE QString urlForGrab(QUrl url) const {return url.toLocalFile();}

signals:
    void typeChanged();
};

}
#endif // SELECT_H
