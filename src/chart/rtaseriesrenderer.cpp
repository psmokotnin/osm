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
#include <cmath>
#include "rtaseriesrenderer.h"
#include "rtaplot.h"

using namespace Fftchart;
RTASeriesRenderer::RTASeriesRenderer() :
    m_pointsPerOctave(0),
    m_mode(0),
    m_coherence(false), m_coherenceThreshold(0)
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/logx.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/color.frag");

    m_program.link();
    m_posAttr       = m_program.attributeLocation("posAttr");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
}
void RTASeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<RTAPlot*>(m_item->parent())) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_mode = plot->mode();
        //TODO: may be remove
        //m_coherence = plot->coherence();
        //m_coherenceThreshold = plot->coherenceThreshold();
    }
}
void RTASeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->size())
        return;

    QMatrix4x4 matrix;

    matrix.ortho(0, 1, yMax, yMin, -1, 1);
    matrix.scale(1  / logf(xMax / xMin), 1.0f, 1.0f);
    matrix.translate(-1 * logf(xMin), 0);
    m_program.setUniformValue(m_matrixUniform, matrix);
    openGLFunctions->glLineWidth(m_weight * m_retinaScale);

    switch (m_mode)
    {
        //line
        case 0:
            renderLine();
        break;

        //bars
        case 1:
            renderBars();
        break;

        //lines
        case 2:
            renderLines();
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

    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));
    openGLFunctions->glEnableVertexAttribArray(0);

    for (unsigned int i = 1, j = 0; i < count; ++i, j += 2) {
        vertices[2] = m_source->frequency(i);
        vertices[3] = 20 * log10f(m_source->module(i) *.1f);
        if (m_coherence) {
            m_program.setUniformValue(
                m_colorUniform,
                static_cast<GLfloat>(m_source->color().redF()),
                static_cast<GLfloat>(m_source->color().greenF()),
                static_cast<GLfloat>(m_source->color().blueF()),
                static_cast<GLfloat>(m_coherence ?
                                     (m_source->coherence(i) > m_coherenceThreshold ? m_source->coherence(i) : 0.f) :
                                     1.f)
            );
        }
        if (i > 1) {
            openGLFunctions->glDrawArrays(GL_LINE_STRIP, 0, 2);
        }
        vertices[0] = vertices[2];
        vertices[1] = vertices[3];
    }
    openGLFunctions->glDisableVertexAttribArray(0);
}
void RTASeriesRenderer::renderBars()
{//TODO:add coherence
    float value = 0;
    GLfloat vertices[8];

    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));
    openGLFunctions->glEnableVertexAttribArray(0);

    auto accumalte =[m_source = m_source, &value] (unsigned int i)
    {
        value += powf(m_source->module(i) *.1f, 2);
    };

    auto collected = [&value, &vertices, openGLFunctions = openGLFunctions, minValue = yMin]
            (float start, float end, unsigned int count)
    {
        Q_UNUSED(count)

        value = 10 * log10f(value);

        vertices[0] = start;
        vertices[1] = value;
        vertices[2] = start;
        vertices[3] = minValue;
        vertices[4] = end;
        vertices[5] = value;
        vertices[6] = end;
        vertices[7] = minValue;

        openGLFunctions->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        value = 0;
    };

    iterate(m_pointsPerOctave, accumalte, collected);

    openGLFunctions->glDisableVertexAttribArray(0);
}
void RTASeriesRenderer::renderLines()
{//TODO:add coherence
    unsigned int count = m_source->size();
    GLfloat vertices[4];
    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));
    openGLFunctions->glEnableVertexAttribArray(0);
    for (unsigned int i = 0; i < count; ++i) {
        vertices[0] = m_source->frequency(i);
        vertices[1] = yMin;
        vertices[2] = m_source->frequency(i);
        vertices[3] = 20 * log10f(m_source->module(i) *.1f);
        openGLFunctions->glDrawArrays(GL_LINES, 0, 2);
    }
}
