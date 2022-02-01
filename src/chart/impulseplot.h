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
#ifndef IMPULSEPLOT_H
#define IMPULSEPLOT_H

#include "xyplot.h"

namespace chart {
class ImpulsePlot : public XYPlot
{
public:
    enum Mode {
        Linear  = 0x00,
        Log     = 0x01
    };

    Q_OBJECT
    Q_ENUM(Mode);
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    ImpulsePlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);
    virtual void setSettings(Settings *settings) noexcept override;
    virtual void storeSettings() noexcept override;

    Mode mode() const;
    void setMode(const Mode &mode);
    void setMode(const int &mode);

signals:
    void modeChanged(Mode);

protected:
    virtual SeriesItem *createSeriesFromSource(Source *source) override;

private:
    Mode m_mode;

};
}
#endif // IMPULSEPLOT_H
