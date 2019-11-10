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
#ifndef PHASEPLOT_H
#define PHASEPLOT_H

#include "xyplot.h"

namespace Fftchart {
class PhasePlot : public XYPlot
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)
    Q_PROPERTY(int rotate READ rotate WRITE setRotate NOTIFY rotateChanged)
    Q_PROPERTY(int range READ range WRITE setRange NOTIFY rangeChanged)
    Q_PROPERTY(bool coherence READ coherence WRITE setCoherence NOTIFY coherenceChanged)

protected:
    unsigned int m_pointsPerOctave;
    int m_center, m_range, m_width;
    bool m_coherence;
    virtual SeriesFBO* createSeriesFromSource(Source *source) override;

public:
    PhasePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);
    unsigned int pointsPerOctave() {return m_pointsPerOctave;}
    void setPointsPerOctave(unsigned int p);

    int rotate() const noexcept{return m_center;}
    void setRotate(int r) noexcept;

    bool coherence() const noexcept {return  m_coherence;}
    void setCoherence(bool coherence) noexcept;

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    int range() const noexcept {return m_range;}
    void setRange(int range) noexcept;

signals:
    void pointsPerOctaveChanged(unsigned int);
    void rotateChanged(int);
    void rangeChanged(int);
    void coherenceChanged(bool);
};
};
#endif // PHASEPLOT_H
