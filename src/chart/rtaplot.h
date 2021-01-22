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

namespace Fftchart {
class RTAPlot : public FrequencyBasedPlot
{
    Q_OBJECT

    Q_PROPERTY(unsigned int mode READ mode WRITE setMode NOTIFY modeChanged)

protected:
    virtual SeriesFBO *createSeriesFromSource(Source *source) override;
    unsigned int m_mode;
    bool m_spline;

public:
    RTAPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    void setMode(unsigned int mode);
    unsigned int mode()
    {
        return m_mode;
    }

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

signals:
    void modeChanged(unsigned int);
};
}
#endif // RTAPLOT_H
