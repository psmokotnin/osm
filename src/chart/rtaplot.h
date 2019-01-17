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
#ifndef RTAPLOT_H
#define RTAPLOT_H

#include "xyplot.h"

namespace Fftchart {
class RTAPlot : public XYPlot
{
    Q_OBJECT

    Q_PROPERTY(unsigned int mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)

protected:
    virtual SeriesFBO* createSeriesFromSource(Source *source) override;
    unsigned int m_mode;
    unsigned int m_pointsPerOctave;
    bool m_spline;

public:
    RTAPlot(QQuickItem *parent = Q_NULLPTR);

    void setMode(unsigned int mode) {if (m_mode == mode) return; m_mode = mode; emit modeChanged();}
    unsigned int mode() {return m_mode;}

    unsigned int pointsPerOctave() {return m_pointsPerOctave;}
    void setPointsPerOctave(unsigned int p);

signals:
    void modeChanged();
    void pointsPerOctaveChanged();
};
}
#endif // RTAPLOT_H
