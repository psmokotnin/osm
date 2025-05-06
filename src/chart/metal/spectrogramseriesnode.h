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
#ifndef CHART_SPECTROGRAMSERIESNODE_H
#define CHART_SPECTROGRAMSERIESNODE_H

#include "xyseriesnode.h"
#include "../frequencybasedserieshelper.h"
#include <QElapsedTimer>
#include <deque>

namespace Chart {

class SpectrogramSeriesNode : public XYSeriesNode, public FrequencyBasedSeriesHelper
{
    Q_OBJECT

public:
    SpectrogramSeriesNode(QQuickItem *item);
    ~SpectrogramSeriesNode();

protected:
    void initRender() override;
    void synchronizeSeries() override;
    void renderSeries() override;
    void updateMatrix() override;
    const Shared::Source &source() const override;

private slots:
    void updateHistory();

private:
    typedef std::array<float, 5> historyPoint;
    typedef std::vector<historyPoint> historyRowData;
    struct historyRow {
        int time;
        historyRowData data;
    };
    static const unsigned int MAX_HISTORY = 52;
    std::deque<historyRow> m_history;
    bool m_refreshBuffers, m_plotActive;
    int m_min, m_mid, m_max;
    unsigned int m_pointsPerOctave;
    unsigned int m_sourceSize;
    QElapsedTimer m_timer;

    //! MTLRenderPipelineState
    void *m_pipeline;

    //! MTLBuffer
    void *m_indiciesBuffer;
};

} // namespace chart

#endif // CHART_SPECTROGRAMSERIESNODE_H
