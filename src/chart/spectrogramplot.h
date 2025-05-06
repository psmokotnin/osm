/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

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
#ifndef SPECTROGRAMPLOT_H
#define SPECTROGRAMPLOT_H

#include "frequencybasedplot.h"

namespace Chart {
class SpectrogramPlot : public FrequencyBasedPlot
{
    Q_OBJECT
    Q_PROPERTY(int min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(int mid READ mid WRITE setMid NOTIFY midChanged)
    Q_PROPERTY(int max READ max WRITE setMax NOTIFY maxChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    SpectrogramPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);
    void setSettings(Settings *settings) noexcept override;
    void storeSettings() noexcept override;

    int min() const;
    void setMin(int min);

    int mid() const;
    void setMid(int mid);

    int max() const;
    void setMax(int max);

    bool active() const;
    void setActive(bool active);

signals:
    void minChanged(int);
    void midChanged(int);
    void maxChanged(int);
    void activeChanged(bool);

protected:
    virtual SeriesItem *createSeriesFromSource(const Shared::Source &source) override;

    int m_min, m_mid, m_max;
    bool m_active;
};
}
#endif // SPECTROGRAMPLOT_H
