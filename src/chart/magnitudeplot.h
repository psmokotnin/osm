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
#ifndef MAGNITUDEPLOT_H
#define MAGNITUDEPLOT_H

#include "frequencybasedplot.h"

namespace Fftchart {
class MagnitudePlot : public FrequencyBasedPlot
{
    Q_OBJECT
    Q_PROPERTY(bool invert READ invert WRITE setInvert NOTIFY invertChanged)

protected:
    virtual SeriesFBO *createSeriesFromSource(Source *source) override;

    bool m_invert;

public:
    MagnitudePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    bool invert() const;
    void setInvert(bool invert);

signals:
    void invertChanged(bool);
};
}

#endif // MAGNITUDEPLOT_H
