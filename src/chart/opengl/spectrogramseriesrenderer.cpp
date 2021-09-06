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
#include "spectrogramseriesrenderer.h"

#include <QQuickWindow>
#include <cmath>
#include "common/notifier.h"
#include "seriesfbo.h"
#include "../spectrogramplot.h"

using namespace chart;

SpectrogramSeriesRenderer::SpectrogramSeriesRenderer() : FrequencyBasedSeriesRenderer(),
    m_pointsPerOctave(0), m_timer()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/spectrogram.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/spectrogram.frag");

    if (!m_program.link()) {
        emit Notifier::getInstance()->newMessage("SpectrogramSeriesRenderer", m_program.log());
    }
    m_matrixUniform = m_program.uniformLocation("matrix");
}
void SpectrogramSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);
    if (auto *plot = dynamic_cast<SpectrogramPlot *>(m_item->parent())) {
        if (
            m_pointsPerOctave != plot->pointsPerOctave() ||
            m_sourceSize != m_source->size()
        ) {
            m_refreshBuffers = true;
            history.clear();
        }
        m_sourceSize = m_source->size();
        m_pointsPerOctave = plot->pointsPerOctave();
        m_min = plot->min();
        m_mid = plot->mid();
        m_max = plot->max();
    }
}

void SpectrogramSeriesRenderer::updateMatrix()
{
    m_matrix = {};
    m_matrix.ortho(0, 1, m_yMax, m_yMin, -1, 1);
    m_matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    m_matrix.translate(-1 * logf(m_xMin), 0);
}
void SpectrogramSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->size())
        return;

    float floor = -140.f;
    float alpha;

    historyRowData rowData;
    historyRow row;
    row.time = static_cast<int>(m_timer.restart());
    row.data.reserve(m_pointsPerOctave * 11);
    float value = 0.f;
    static const QColor qred("#F44336"), qgreen("#8BC34A"), qblue("#2196F3");
    QColor pointColor;

    auto mix = [] (const QColor & first, const QColor & second, qreal k) {
        QColor mixedColor;
        mixedColor.setRedF(  k * (second.redF()   - first.redF())   + first.redF());
        mixedColor.setBlueF( k * (second.blueF()  - first.blueF())  + first.blueF());
        mixedColor.setGreenF(k * (second.greenF() - first.greenF()) + first.greenF());
        return mixedColor;
    };
    auto accumalte = [m_source = m_source, &value] (const unsigned int &i) {
        if (i == 0) {
            return ;
        }
        value += 2 * m_source->module(i) * m_source->module(i) * (m_source->frequency(i) - m_source->frequency(i - 1));
    };
    auto collected = [&] (const float & start, const float & end, const unsigned int &) {

        value = 10 * log10f(value) + LEVEL_NORMALIZATION;

        alpha = 1.0f;
        if (!std::isnormal(value) || value < floor) {
            value = floor;
        }
        if (value > m_max) {
            value = m_max;
        }

        if (value < m_min) {
            // transparent -> blue
            pointColor = qblue;
            pointColor = qblue;
            alpha = (value - floor) / (m_min   - floor);
        } else if (value < m_mid) {
            // blue -> green
            pointColor = mix(qblue, qgreen, static_cast<qreal>((value - m_min) / (m_mid - m_min)));
        } else  {
            // green -> red
            pointColor = mix(qgreen, qred, static_cast<qreal>((value - m_mid) / (m_max - m_mid)));
        }

        historyPoint rgb;
        rgb[0] = (start + end) / 2.f;
        rgb[1] = static_cast<float>(pointColor.redF());
        rgb[2] = static_cast<float>(pointColor.greenF());
        rgb[3] = static_cast<float>(pointColor.blueF());
        rgb[4] = alpha;
        row.data.push_back(rgb);

        value = 0;
    };

    iterate(m_pointsPerOctave, accumalte, collected);

    //TODO: change to fifo instead of deque
    history.push_back(std::move(row));
    if (history.size() > 51) {
        history.pop_front();
    }

    unsigned int maxBufferSize = 51 * (m_pointsPerOctave * 12 + 4) * 6,
                 verticiesCount = 0, maxIndicesCount = 0, indicesCount = 0;
    if (m_vertices.size() != maxBufferSize) {
        m_vertices.resize(maxBufferSize);
        m_refreshBuffers = true;
    }

    auto rowSize = history[0].data.size();
    maxIndicesCount = 51 * (2 * rowSize + 2);
    if (m_indices.size() != maxIndicesCount) {
        m_indices.resize(maxIndicesCount);
        m_refreshBuffers = true;
    }

    float t(0), tStep(0);
    unsigned int j = 0, index = 0;
    auto addPoint = [&](const historyPoint & data, const float & time) {
        if (j > maxBufferSize) {
            qCritical("out of range");
            return;
        }
        m_vertices[j + 0] = data[0];
        m_vertices[j + 1] = time;
        m_vertices[j + 2] = data[1];
        m_vertices[j + 3] = data[2];
        m_vertices[j + 4] = data[3];
        m_vertices[j + 5] = data[4];
        j += 6;
        verticiesCount ++;
    };

    for (auto row = history.crbegin(); row != history.crend(); ++row) {

        const historyRowData *rowData = &(row->data);
        tStep = row->time / 1000.f;

        if (rowData->size() != rowSize) {
            t += tStep;
            continue;
        }
        unsigned int i = 0;
        m_indices[indicesCount++] = index + i;
        for (; i < rowData->size(); ++i) {
            addPoint(rowData->at(i), t);
            m_indices[indicesCount++] = index + i;
            m_indices[indicesCount++] = index + i + rowSize;
        }
        m_indices[indicesCount++] = index + (i - 1) + rowSize;
        t += tStep;
        index += rowSize;
    }
    indicesCount--;

    m_program.setUniformValue(m_matrixUniform, m_matrix);
    m_openGLFunctions->glLineWidth(m_weight * m_retinaScale);

    if (m_refreshBuffers) {
        m_openGLFunctions->glGenBuffers(1, &m_vertexBufferId);
        m_openGLFunctions->glGenVertexArrays(1, &m_vertexArrayId);
        m_openGLFunctions->glGenBuffers(1, &m_indexBufferId);
    }

    m_openGLFunctions->glBindVertexArray(m_vertexArrayId);
    m_openGLFunctions->glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    m_openGLFunctions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);

    if (m_refreshBuffers) {
        m_openGLFunctions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * maxIndicesCount, nullptr, GL_DYNAMIC_DRAW);
        m_openGLFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBufferSize, nullptr, GL_DYNAMIC_DRAW);
        m_openGLFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(0));
        m_openGLFunctions->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                                                 reinterpret_cast<const void *>(2 * sizeof(GLfloat)));
    }
    m_openGLFunctions->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLfloat) * maxIndicesCount, m_indices.data());
    m_openGLFunctions->glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(GLfloat) * verticiesCount, m_vertices.data());

    m_openGLFunctions->glEnableVertexAttribArray(0);
    m_openGLFunctions->glEnableVertexAttribArray(1);
    m_openGLFunctions->glDrawElements(GL_TRIANGLE_STRIP, indicesCount, GL_UNSIGNED_INT, 0);
    m_openGLFunctions->glDisableVertexAttribArray(1);
    m_openGLFunctions->glDisableVertexAttribArray(0);

    m_refreshBuffers = false;
}
