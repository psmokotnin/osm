/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

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
#include "rtaseriesrenderer.h"
#include "rtaplot.h"

using namespace Fftchart;
RTASeriesRenderer::RTASeriesRenderer() : FrequencyBasedSeriesRenderer()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/logx.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/color.frag");

    m_program.link();
    m_posAttr       = m_program.attributeLocation("posAttr");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
}
void RTASeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    RTAPlot *plot = static_cast<RTAPlot*>(m_item->parent());
    QMatrix4x4 matrix;

    matrix.ortho(0, 1, plot->yAxis()->max(), plot->yAxis()->min(), -1, 1);
    matrix.scale(1  / logf(plot->xAxis()->max() / plot->xAxis()->min()), 1.0f, 1.0f);
    matrix.translate(-1 * logf(plot->xAxis()->min()), 0);
    m_program.setUniformValue(m_matrixUniform, matrix);
    openGLFunctions->glLineWidth(2);

    switch (plot->mode())
    {
        //line
        case 0:
            renderLine();
        break;

        //bars
        case 1:
            renderBars(plot);
        break;

        //lines
        case 2:
            renderLines(plot);
        break;

        default:
            return;
    }
    openGLFunctions->glDisableVertexAttribArray(0);
}
void RTASeriesRenderer::renderLine()
{
    unsigned int count = m_source->size();
    GLfloat vertices[4];

    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, vertices);
    openGLFunctions->glEnableVertexAttribArray(0);

    for (unsigned int i = 1, j = 0; i < count; ++i, j += 2) {
        vertices[2] = m_source->frequency(i);
        vertices[3] = m_source->module(i);
        if (i > 1) {
            openGLFunctions->glDrawArrays(GL_LINE_STRIP, 0, 2);
        }
        vertices[0] = vertices[2];
        vertices[1] = vertices[3];
    }
    openGLFunctions->glDisableVertexAttribArray(0);
}
void RTASeriesRenderer::renderBars(RTAPlot *plot)
{
    float value = 0,
          minValue = plot->yAxis()->min();
    GLfloat vertices[8];

    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, vertices);
    openGLFunctions->glEnableVertexAttribArray(0);

    auto accumalte =[m_source = m_source, &value] (unsigned int i)
    {
        value += powf(m_source->dataAbs(i) / m_source->fftSize(), 2);
    };

    auto collected = [&minValue, &value, &vertices, openGLFunctions = openGLFunctions]
            (float start, float end, unsigned int count)
    {
        Q_UNUSED(count)

        value = 10 * log10f(value);

        vertices[0] = start;
        vertices[1] = minValue;
        vertices[2] = start;
        vertices[3] = value;
        vertices[4] = end;
        vertices[5] = value;
        vertices[6] = end;
        vertices[7] = minValue;

        openGLFunctions->glDrawArrays(GL_QUADS, 0, 4);
        value = 0;
    };

    iterate(plot->pointsPerOctave(), accumalte, collected);

    openGLFunctions->glDisableVertexAttribArray(0);
}
void RTASeriesRenderer::renderLines(RTAPlot *plot)
{
    unsigned int count = m_source->size();
    GLfloat vertices[4];
    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, vertices);
    openGLFunctions->glEnableVertexAttribArray(0);
    for (unsigned int i = 0; i < count; ++i) {
        vertices[0] = m_source->frequency(i);
        vertices[1] = plot->yAxis()->min();
        vertices[2] = m_source->frequency(i);
        vertices[3] = m_source->module(i);
        openGLFunctions->glDrawArrays(GL_LINES, 0, 2);
    }
}
