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
#ifndef CHART_IMPULSESERIESNODE_H
#define CHART_IMPULSESERIESNODE_H

#include "xyseriesnode.h"
#include "../impulseplot.h"

namespace Chart {

class ImpulseSeriesNode : public XYSeriesNode
{
    Q_OBJECT
public:
    ImpulseSeriesNode(QQuickItem *item);
    ~ImpulseSeriesNode();

protected:
    void initRender() override;
    void synchronizeSeries() override;
    void renderSeries() override;
    void updateMatrix() override;

private:
    //! MTLRenderPipelineState
    void *m_pipeline;

    ImpulsePlot::Mode m_mode;
    bool m_normalized;
};

} // namespace chart

#endif // CHART_IMPULSESERIESNODE_H
