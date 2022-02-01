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

#include "frequencybasedplot.h"

namespace chart {
class RTAPlot : public FrequencyBasedPlot
{
    Q_OBJECT
    Q_PROPERTY(unsigned int mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool showPeaks READ showPeaks WRITE setShowPeaks NOTIFY showPeaksChanged)

public:
    RTAPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    void setMode(unsigned int mode);
    unsigned int mode();

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    bool showPeaks() const;
    void setShowPeaks(bool showPeaks);

signals:
    void modeChanged(unsigned int);
    void showPeaksChanged(bool);

private:
    virtual SeriesItem *createSeriesFromSource(Source *source) override;
    unsigned int m_mode;
    bool m_spline, m_showPeaks;
};
}
#endif // RTAPLOT_H
