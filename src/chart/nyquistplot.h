/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
#ifndef CHART_NYQUISTPLOT_H
#define CHART_NYQUISTPLOT_H

#include "frequencybasedplot.h"
namespace Chart {

class NyquistPlot : public FrequencyBasedPlot
{
    Q_OBJECT

public:
    NyquistPlot(Settings *settings, QQuickItem *parent = Q_NULLPTR);

    void setSettings(Settings *settings) noexcept override;
    void storeSettings() noexcept override;

    struct SplineValue {
        Complex m_phase = 0;
        float m_magnitude = 0;

        SplineValue(Complex phase, float magnitude) : m_phase(phase), m_magnitude(magnitude) {}
        SplineValue(const SplineValue &right)
        {
            m_phase = right.m_phase;
            m_magnitude = right.m_magnitude;
        };

        SplineValue(SplineValue &&right) noexcept
        {
            m_phase = std::move(right.m_phase);
            m_magnitude = std::move(right.m_magnitude);
        };

        SplineValue &operator=(const SplineValue &rh)
        {
            m_phase = rh.m_phase;
            m_magnitude = rh.m_magnitude;
            return *this;
        }

        SplineValue &operator+=(const Complex &rh)
        {
            m_phase += rh;
            return *this;
        }

        SplineValue &operator+=(const float &rh)
        {
            m_magnitude += rh;
            return *this;
        }

        void reset()
        {
            m_phase = 0;
            m_magnitude = 0;
        }
    };

protected:
    SeriesItem *createSeriesFromSource(const Shared::Source &source) override;
};

} // namespace chart

#endif // CHART_NYQUISTPLOT_H
