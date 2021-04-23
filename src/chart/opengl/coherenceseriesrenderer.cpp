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
#include "coherenceseriesrenderer.h"

using namespace chart;

CoherenceSeriesRenderer::CoherenceSeriesRenderer() : FrequencyBasedSeriesRenderer(), m_pointsPerOctave(24)
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/coherence.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/coherence.geom");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/coherence.frag");
    m_program.link();

    m_widthUniform  = m_program.uniformLocation("width");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
    m_minmaxUniform = m_program.uniformLocation("minmax");
    m_screenUniform = m_program.uniformLocation("screen");
}
void CoherenceSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<CoherencePlot *>(m_item->parent())) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_type = plot->type();
    }
}
void CoherenceSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->size())
        return;

    unsigned int maxBufferSize = m_pointsPerOctave * 11 * 8, i = 0, verticiesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    float value = 0.f, coherence = 0.f;

    float xadd, xmul;
    xadd = -1.0f * logf(m_xMin);
    xmul = m_width / logf(m_xMax / m_xMin);

    /*
     * Draw quad for each band from min to max (full height)
     * pass spline data to shaders
     * fragment shader draws spline function
     */
    auto accumulate = [this, &value] (const unsigned int &i) {
        value += (m_type == CoherencePlot::Type::Squared ?
                  powf(m_source->coherence(i), 2) :
                  m_source->coherence(i)
                 );
    };

    auto collected = [ &, this]
    (const float & f1, const float & f2, const float * ac, const float *) {
        if (i > maxBufferSize) {
            qCritical("out of range");
            return;
        }
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;

        m_vertices[i + 0] = f1;
        m_vertices[i + 1] = f2;
        m_vertices[i + 2] = fx1;
        m_vertices[i + 3] = fx2;
        std::memcpy(m_vertices.data() + i + 4, ac, 4 * 4);
        verticiesCount ++;
        i += 8;
        value = 0.f;
        coherence = 0.f;
    };

    iterateForSpline<float, float>(m_pointsPerOctave, &value, &coherence, accumulate, collected);

    setUniforms();

    if (m_refreshBuffers) {
        m_openGLFunctions->glGenBuffers(1, &m_vertexBufferId);
        m_openGLFunctions->glGenVertexArrays(1, &m_vertexArrayId);
    }

    m_openGLFunctions->glBindVertexArray(m_vertexArrayId);
    m_openGLFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

    if (m_refreshBuffers) {
        m_openGLFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
        m_openGLFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(0));
        m_openGLFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(2 * sizeof(GLfloat)));
        m_openGLFunctions->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(4 * sizeof(GLfloat)));
    }
    m_openGLFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

    m_openGLFunctions->glEnableVertexAttribArray(0);
    m_openGLFunctions->glEnableVertexAttribArray(1);
    m_openGLFunctions->glEnableVertexAttribArray(2);
    m_openGLFunctions->glDrawArrays(GL_POINTS, 0, verticiesCount);
    m_openGLFunctions->glDisableVertexAttribArray(2);
    m_openGLFunctions->glDisableVertexAttribArray(1);
    m_openGLFunctions->glDisableVertexAttribArray(0);

    m_refreshBuffers = false;
}
