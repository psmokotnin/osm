/**
 *  OSM
 *  Copyright (C) 2020  Pavel Smokotnin

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
#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include "frequencybasedseriesrenderer.h"

namespace Fftchart {
class SpectrogramSeriesRenderer : public FrequencyBasedSeriesRenderer
{
public:
    explicit SpectrogramSeriesRenderer();
    void renderSeries() override;
    void synchronize(QQuickFramebufferObject *item) override;

protected:
    typedef std::array<float, 6> historyPoint;
    typedef std::vector<historyPoint> historyRowData;
    struct historyRow {
        int time;
        historyRowData data;
    };
    std::deque<historyRow> history;

private:
    int m_posAttr, m_prePositionAttr, m_postPositionAttr;
    int m_colorLeftUniform, m_colorRightUniform;
    unsigned int m_pointsPerOctave;
    QElapsedTimer m_timer;
};
}
#endif // SPECTROGRAMSERIESRENDERER_H
