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
#ifndef CHART_XYSERIESNODE_H
#define CHART_XYSERIESNODE_H

#include "seriesnode.h"

namespace Chart {

class XYSeriesNode : public SeriesNode
{
    Q_OBJECT

public:
    XYSeriesNode(QQuickItem *item);
    virtual ~XYSeriesNode();

protected:
    virtual void updateMatrix() = 0;
    void synchronizeMatrix();

    void *initLine(QString vertexProgramName = "logLineVertex");
    float *vertexBuffer(unsigned int maxBufferSize);
    void encodeLine(void *pipeline, unsigned int verticiesCount);
    float coherenceSpline(const bool &coherence, const float &threshold, const float data[4], const float &t) const;
    void addLinePoint(float *vertex_ptr,
                      unsigned int &i, unsigned int &verticiesCount, const float &x, const float &y,
                      const float &dX, const float &dY, const float &d);
    void addLineSegment(float *vertex_ptr, unsigned int &i, unsigned int &verticiesCount,
                        const float &fromX, const float &fromY,
                        const float &toX, const float &toY,
                        const float &fromC, const float &toC);

    //! max point count for spliting each spline segment
    static const unsigned int MAX_LINE_SPLIT = 40;
    constexpr static const float MAX_LINE_SPLITF = static_cast<float>(MAX_LINE_SPLIT);
    static const unsigned int LINE_VERTEX_SIZE = 5;
    static const unsigned int VERTEX_PER_SEGMENT = 6;
    constexpr static const unsigned int PPO_BUFFER_MUL = 13 * VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE * MAX_LINE_SPLITF;

    QMatrix4x4 m_matrix;
    float m_xMin, m_xMax, m_yMin, m_yMax;

    //!MTLBuffer
    void *m_vertexBuffer, *m_matrixBuffer;
};

} // namespace chart

#endif // CHART_XYSERIESNODE_H
