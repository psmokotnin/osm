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
#ifndef XYSERIESRENDERER_H
#define XYSERIESRENDERER_H

#include "seriesrenderer.h"

namespace Chart {
class XYSeriesRenderer : public SeriesRenderer
{
public:
    XYSeriesRenderer(bool logX, bool logY);
    virtual void synchronize(QQuickFramebufferObject *item) override;

protected:
    virtual void updateMatrix() = 0;

    QMatrix4x4 m_matrix;
    int m_matrixUniform;
    float m_xMin, m_xMax, m_yMin, m_yMax;
    bool m_logX, m_logY;
};
}

#endif // XYSERIESRENDERER_H
