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
#ifndef CHART_MAGNITUDESERIESNODE_H
#define CHART_MAGNITUDESERIESNODE_H

#include "xyseriesnode.h"
#include "../frequencybasedserieshelper.h"
#include "../magnitudeplot.h"

namespace chart {

class MagnitudeSeriesNode : public XYSeriesNode, public FrequencyBasedSeriesHelper
{
    Q_OBJECT

public:
    MagnitudeSeriesNode(QQuickItem *item);
    ~MagnitudeSeriesNode();

protected:
    void initRender() override;
    void synchronizeSeries() override;
    void renderSeries() override;
    void updateMatrix() override;
    Source *source() const override;

private:
    unsigned int m_pointsPerOctave;
    float m_coherenceThreshold, m_sensor;
    bool m_coherence, m_invert;

    //! MTLRenderPipelineState
    void *m_pipeline;

    MagnitudePlot::Mode m_mode;
};
}
#endif // CHART_MAGNITUDESERIESNODE_H
