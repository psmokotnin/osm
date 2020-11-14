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
#include "stepseriesrenderer.h"

using namespace Fftchart;

StepSeriesRenderer::StepSeriesRenderer()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/pos.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/color.frag");
    m_program.link();
    m_posAttr = m_program.attributeLocation("posAttr");
    m_colorUniform = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
}

void StepSeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    QMatrix4x4 matrix;
    GLfloat vertices[4];
    float res = 0.f;

    matrix.ortho(xMin, xMax, yMax, yMin, -1, 1);
    m_program.setUniformValue(m_matrixUniform, matrix);

    auto offsetPoint = m_source->impulseSize() / 2 - 1;

    float offsetValue = 0.f;
    for (unsigned int i = 1; i < offsetPoint; ++i) {
        offsetValue += m_source->impulseValue(i);
    }

    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, static_cast<const void *>(vertices));

    openGLFunctions->glEnableVertexAttribArray(0);
    openGLFunctions->glLineWidth(2 * m_retinaScale);

    for (unsigned int i = 1, j = 0; i <= m_source->impulseSize() - 1; ++i, j += 2) {
        res += m_source->impulseValue(i);
        vertices[2] = m_source->impulseTime(i);
        vertices[3] = res - offsetValue;

        if (i > 1) {
            openGLFunctions->glDrawArrays(GL_LINE_STRIP, 0, 2);
        }
        vertices[0] = vertices[2];
        vertices[1] = vertices[3];
    }

    openGLFunctions->glDisableVertexAttribArray(0);
}

