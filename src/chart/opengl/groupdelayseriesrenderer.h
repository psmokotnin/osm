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
#ifndef GROUPDELAYSERIESRENDERER_H
#define GROUPDELAYSERIESRENDERER_H

#include "frequencybasedseriesrenderer.h"

namespace Chart {
class GroupDelaySeriesRenderer : public FrequencyBasedSeriesRenderer
{
public:
    GroupDelaySeriesRenderer();
    void init() override;
    void renderSeries() override;
    void synchronize(QQuickFramebufferObject *item) override;

private:
    int m_coherenceThresholdU, m_coherenceAlpha;
    unsigned int m_pointsPerOctave;
    float m_coherenceThreshold;
    bool m_coherence;
};
}
#endif // GROUPDELAYSERIESRENDERER_H
