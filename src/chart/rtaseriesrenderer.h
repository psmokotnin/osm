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
#ifndef RTASERIES_H
#define RTASERIES_H

#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>

#include "frequencybasedseriesrenderer.h"
#include "rtaplot.h"

namespace Fftchart {

class RTASeriesRenderer : public FrequencyBasedSeriesRenderer
{
public:
    explicit RTASeriesRenderer();
    void renderSeries() override;
    void synchronize(QQuickFramebufferObject *item) override;

protected:
    void renderLine();
    void renderLines();
    void renderBars();

    void drawVertices(const GLenum &mode, const GLsizei &count);
    virtual void updateMatrix() override;

private:
    void initShaders();

    unsigned int m_pointsPerOctave, m_mode;
    QOpenGLShader m_vertexShader, m_geometryShader, m_fragmentShader;
};
}
#endif // RTASERIES_H
