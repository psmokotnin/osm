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
#ifndef CHART_CRESTFACTORSERIESNODE_H
#define CHART_CRESTFACTORSERIESNODE_H

#include "xyseriesnode.h"
#include "../frequencybasedserieshelper.h"
#include "../crestfactorplot.h"

namespace Chart {

class CrestFactorSeriesNode : public XYSeriesNode, public FrequencyBasedSeriesHelper
{
    Q_OBJECT

public:
    CrestFactorSeriesNode(QQuickItem *item);
    ~CrestFactorSeriesNode();

protected:
    void initRender() override;
    void synchronizeSeries() override;
    void renderSeries() override;
    void updateMatrix() override;
    const Shared::Source &source() const override;

private:
    unsigned int m_pointsPerOctave;

    //! MTLRenderPipelineState
    void *m_pipeline;
};

} // namespace chart

#endif // CHART_CRESTFACTORSERIESNODE_H
