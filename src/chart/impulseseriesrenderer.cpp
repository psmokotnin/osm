/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
#include "impulseseriesrenderer.h"
#include "impulseplot.h"

using namespace Fftchart;

ImpulseSeriesRenderer::ImpulseSeriesRenderer() : SeriesRenderer()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/pos.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/color.frag");
    m_program.link();
    m_posAttr = m_program.attributeLocation("posAttr");
    m_colorUniform = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
}
void ImpulseSeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    ImpulsePlot *plot = static_cast<ImpulsePlot *>(m_item->parent());
    QMatrix4x4 matrix;
    GLfloat vertices[4];

    matrix.ortho(plot->xAxis()->min(), plot->xAxis()->max(), plot->yAxis()->max(), plot->yAxis()->min(), -1, 1);
    m_program.setUniformValue(m_matrixUniform, matrix);

    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, vertices);

    openGLFunctions->glEnableVertexAttribArray(0);
    openGLFunctions->glLineWidth(2);

    for (unsigned int i = 1, j = 0; i <= m_source->impulseSize() - 1; ++i, j += 2) {
        vertices[2] = m_source->impulseTime(i);
        vertices[3] = m_source->impulseValue(i);

        if (i > 1) {
            openGLFunctions->glDrawArrays(GL_LINE_STRIP, 0, 2);
        }
        vertices[0] = vertices[2];
        vertices[1] = vertices[3];
    }

    openGLFunctions->glDisableVertexAttribArray(0);
}
