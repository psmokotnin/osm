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
#ifndef SPECTROGRAMSERIESRENDERER_H
#define SPECTROGRAMSERIESRENDERER_H

#include <deque>
#include <array>
#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include "frequencybasedseriesrenderer.h"

namespace Chart {
class SpectrogramSeriesRenderer : public FrequencyBasedSeriesRenderer
{
public:
    explicit SpectrogramSeriesRenderer();
    void init() override;
    void renderSeries() override;
    void synchronize(QQuickFramebufferObject *item) override;

protected:
    virtual void updateMatrix() override;
    typedef std::array<float, 5> historyPoint;
    typedef std::vector<historyPoint> historyRowData;
    struct historyRow {
        int time;
        historyRowData data;
    };
    std::deque<historyRow> history;

private:
    int m_min, m_mid, m_max;
    unsigned int m_pointsPerOctave;
    QElapsedTimer m_timer;

    unsigned int m_indexBufferId, m_sourceSize;
    std::vector<unsigned int> m_indices;
    bool m_active;
};
}
#endif // SPECTROGRAMSERIESRENDERER_H
