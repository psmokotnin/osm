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
#include "xyseriesrenderer.h"
#include "../xyplot.h"

using namespace Chart;

XYSeriesRenderer::XYSeriesRenderer() : SeriesRenderer(),
    m_matrixUniform(-1),
    m_xMin(0.f),
    m_xMax(0.f),
    m_yMin(0.f),
    m_yMax(0.f)
{

}
void XYSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    SeriesRenderer::synchronize(item);

    if (auto *xyplot = dynamic_cast<XYPlot *>(plot())) {
        if (
            m_xMin != xyplot->xAxis()->min() ||
            m_xMax != xyplot->xAxis()->max() ||
            m_yMin != xyplot->yAxis()->min() ||
            m_yMax != xyplot->yAxis()->max()
        ) {

            m_xMin = xyplot->xAxis()->min();
            m_xMax = xyplot->xAxis()->max();
            m_yMin = xyplot->yAxis()->min();
            m_yMax = xyplot->yAxis()->max();
            updateMatrix();
        }
    }
}
