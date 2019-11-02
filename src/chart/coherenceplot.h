/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#ifndef COHERENCEPLOT_H
#define COHERENCEPLOT_H

#include "xyplot.h"

namespace Fftchart {
class CoherencePlot : public XYPlot
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)

public:
    enum Type {NORMAL, SQUARED};
    Q_ENUMS(Type)

protected:
    unsigned int m_pointsPerOctave;
    Type m_type;
    virtual SeriesFBO* createSeriesFromSource(Source *source) override;

public:
    CoherencePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    unsigned int pointsPerOctave() {return m_pointsPerOctave;}
    void setPointsPerOctave(unsigned int p);

    Type type() {return m_type;}
    void setType(Type);
    void setType(QVariant type) {setType(static_cast<Type>(type.toInt()));}

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

signals:
    void pointsPerOctaveChanged(unsigned int);
    void typeChanged(Type);
};
}
#endif // COHERENCEPLOT_H
