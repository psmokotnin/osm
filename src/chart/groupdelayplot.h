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
#ifndef GROUPDELAYPLOT_H
#define GROUPDELAYPLOT_H

#include "xyplot.h"

namespace Fftchart {
class GroupDelayPlot : public XYPlot
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pointsPerOctave READ pointsPerOctave WRITE setPointsPerOctave NOTIFY pointsPerOctaveChanged)
    Q_PROPERTY(bool coherence READ coherence WRITE setCoherence NOTIFY coherenceChanged)

protected:
    unsigned int m_pointsPerOctave;
    bool m_coherence;
    virtual SeriesFBO* createSeriesFromSource(Source *source) override;

public:
    GroupDelayPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);
    unsigned int pointsPerOctave() {return m_pointsPerOctave;}
    void setPointsPerOctave(unsigned int p);

    bool coherence() const noexcept {return  m_coherence;}
    void setCoherence(bool coherence) noexcept;

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

signals:
    void pointsPerOctaveChanged(unsigned int);
    void coherenceChanged(bool);
};
};
#endif // GROUPDELAYPLOT_H
