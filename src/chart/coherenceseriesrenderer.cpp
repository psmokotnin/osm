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
#include "coherenceseriesrenderer.h"

using namespace Fftchart;

CoherenceSeriesRenderer::CoherenceSeriesRenderer()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/logx.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/color.frag");

    m_program.link();
    m_posAttr       = m_program.attributeLocation("posAttr");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
}
void CoherenceSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);
}
void CoherenceSeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    QMatrix4x4 matrix;

    matrix.ortho(0, 1, yMax, yMin, -1, 1);
    matrix.scale(1  / logf(xMax / xMin), 1.0f, 1.0f);
    matrix.translate(-1 * logf(xMin), 0);
    m_program.setUniformValue(m_matrixUniform, matrix);
    openGLFunctions->glLineWidth(2);

    unsigned int count = m_source->size();
    GLfloat vertices[4];

    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));
    openGLFunctions->glEnableVertexAttribArray(0);

    for (unsigned int i = 1, j = 0; i < count; ++i, j += 2) {
        vertices[2] = m_source->frequency(i);
        vertices[3] = m_source->coherence(i);
        if (vertices[3] >= 1.f) vertices[3] = 1.f - std::numeric_limits<float>::min();
        if (i > 1) {
            openGLFunctions->glDrawArrays(GL_LINE_STRIP, 0, 2);
        }
        vertices[0] = vertices[2];
        vertices[1] = vertices[3];
    }
    openGLFunctions->glDisableVertexAttribArray(0);
}
