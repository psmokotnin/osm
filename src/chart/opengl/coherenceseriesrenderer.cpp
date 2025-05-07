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
#include "common/notifier.h"
using namespace Chart;

CoherenceSeriesRenderer::CoherenceSeriesRenderer() : FrequencyBasedSeriesRenderer(), m_pointsPerOctave(24)
{
}
void CoherenceSeriesRenderer::init()
{
    if (m_openGL33CoreFunctions) {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/coherence.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/coherence.geom");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/coherence.frag");
    } else {
        m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl2/logx.vert");
        m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl2/color.frag");
    }
    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("CoherenceSeriesRenderer", m_program.log());
    }

    if (m_openGL33CoreFunctions) {
        m_widthUniform  = m_program.uniformLocation("width");
        m_colorUniform  = m_program.uniformLocation("m_color");
        m_matrixUniform = m_program.uniformLocation("matrix");
        m_minmaxUniform = m_program.uniformLocation("minmax");
        m_screenUniform = m_program.uniformLocation("screen");
    } else {
        m_positionAttribute = m_program.attributeLocation("position");
        m_colorUniform  = m_program.attributeLocation("color");
        m_matrixUniform = m_program.uniformLocation("matrix");
    }
}
void CoherenceSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);

    if (auto *plot = dynamic_cast<CoherencePlot *>(m_item ? m_item->parent() : nullptr)) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_type = plot->type();
    }
}
void CoherenceSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->frequencyDomainSize())
        return;

    unsigned int maxBufferSize = m_pointsPerOctave * 12 * (m_openGL33CoreFunctions ? 8 : PPO_BUFFER_MUL), i = 0,
                 verticiesCount = 0;
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
        switch (m_type) {
        case CoherencePlot::Type::Squared:
            value += powf(m_source->coherence(i), 2);
            break;
        case CoherencePlot::Type::Normal:
            value += m_source->coherence(i);
            break;
        case CoherencePlot::Type::SNR: {
            auto squared = powf(m_source->coherence(i), 2);
            value += 10.f * log10f(squared / (1 - squared));
        }
        break;

        }
    };

    auto collected = [ &, this]
    (const float & f1, const float & f2, const float * ac, const float *) {
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;
        if (m_openGL33CoreFunctions) {
            if (i + 8 > maxBufferSize) {
                return;
            }
            m_vertices[i + 0] = f1;
            m_vertices[i + 1] = f2;
            m_vertices[i + 2] = fx1;
            m_vertices[i + 3] = fx2;
            std::memcpy(m_vertices.data() + i + 4, ac, 4 * 4);
            verticiesCount ++;
            i += 8;
        } else {
            auto points = std::min(MAX_LINE_SPLITF, std::abs(std::round(fx2 - fx1)));
            float dt = 1.f / points;

            for (float t = 0; t < 1.0;) {

                auto x1 = f1 * std::pow(f2 / f1, t);
                auto v1 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;
                t += dt;
                auto x2 = f1 * std::pow(f2 / f1, t);
                auto v2 = ac[0] + ac[1] * t + ac[2] * t * t + ac[3] * t * t * t;

                addLineSegment(i, verticiesCount,
                               x1, v1,
                               x2, v2,
                               1.f, 1.f);
            }
        }
        value = 0.f;
        coherence = 0.f;
    };

    iterateForSpline<float, float>(m_pointsPerOctave, &value, &coherence, accumulate, collected);

    setUniforms();
    if (m_openGL33CoreFunctions) {
        if (m_refreshBuffers) {
            m_openGL33CoreFunctions->glGenBuffers(1, &m_vertexBufferId);
            m_openGL33CoreFunctions->glGenVertexArrays(1, &m_vertexArrayId);
        }

        m_openGL33CoreFunctions->glBindVertexArray(m_vertexArrayId);
        m_openGL33CoreFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

        if (m_refreshBuffers) {
            m_openGL33CoreFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
            m_openGL33CoreFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(0));
            m_openGL33CoreFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(2 * sizeof(GLfloat)));
            m_openGL33CoreFunctions->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                                                           reinterpret_cast<const void *>(4 * sizeof(GLfloat)));
        }
        m_openGL33CoreFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

        m_openGL33CoreFunctions->glEnableVertexAttribArray(0);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(1);
        m_openGL33CoreFunctions->glEnableVertexAttribArray(2);
        m_openGL33CoreFunctions->glDrawArrays(GL_POINTS, 0, verticiesCount);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(2);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(1);
        m_openGL33CoreFunctions->glDisableVertexAttribArray(0);

        m_refreshBuffers = false;
    } else {
        drawOpenGL2(verticiesCount);
    }
}
