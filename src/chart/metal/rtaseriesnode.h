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
#ifndef CHART_RTASERIES_H
#define CHART_RTASERIES_H

#include "seriesitem.h"
#include "xyseriesnode.h"
#include "../frequencybasedserieshelper.h"
#include "../rtaplot.h"

namespace Chart {

class RTASeriesNode : public XYSeriesNode, public FrequencyBasedSeriesHelper
{
    Q_OBJECT

public:
    RTASeriesNode(QQuickItem *item);
    ~RTASeriesNode();

protected:
    void initRender() override;
    void synchronizeSeries() override;
    void renderSeries() override;
    void updateMatrix() override;
    const Shared::Source &source() const override;

private:
    void renderLine();
    void renderBars();
    void renderLines();

    unsigned int m_pointsPerOctave;
    RTAPlot::Mode m_mode;
    RTAPlot::Scale m_scale;
    static constexpr float absolute_scale_offset = 140.f;

    bool m_refreshBuffers, m_showPeaks;
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indicies;

    //! MTLRenderPipelineState
    void *m_pipelineLine, *m_pipelineBars;

    //!MTLBuffer
    void *m_vertexBuffer, *m_matrixBuffer;
};

} // namespace chart

#endif // CHART_RTASERIES_H
