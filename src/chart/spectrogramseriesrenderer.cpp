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
#include "spectrogramseriesrenderer.h"

#include <QQuickWindow>
#include <cmath>

#include "seriesfbo.h"
#include "spectrogramplot.h"

using namespace Fftchart;

SpectrogramSeriesRenderer::SpectrogramSeriesRenderer():
    m_pointsPerOctave(0), m_timer()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/spectrogram.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/interpolateColor.frag");

    m_program.link();
    m_posAttr       = m_program.attributeLocation("posAttr");
    m_prePositionAttr    = m_program.uniformLocation("prePosition");
    m_postPositionAttr    = m_program.uniformLocation("postPosition");
    m_colorLeftUniform   = m_program.uniformLocation("m_colorLeft");
    m_colorRightUniform  = m_program.uniformLocation("m_colorRight");
    m_screenUniform = m_program.uniformLocation("screen");
    m_matrixUniform = m_program.uniformLocation("matrix");
}
void SpectrogramSeriesRenderer::synchronize(QQuickFramebufferObject *item)
{
    XYSeriesRenderer::synchronize(item);
    if (auto *plot = dynamic_cast<SpectrogramPlot *>(m_item->parent())) {
        m_pointsPerOctave = plot->pointsPerOctave();
        m_min = plot->min();
        m_mid = plot->mid();
        m_max = plot->max();
    }
}
void SpectrogramSeriesRenderer::renderSeries()
{
    if (!m_source->active() || !m_source->size())
        return;

    QMatrix4x4 matrix;

    matrix.ortho(0, 1, m_yMax, m_yMin, -1, 1);
    matrix.scale(1  / logf(m_xMax / m_xMin), 1.0f, 1.0f);
    matrix.translate(-1 * logf(m_xMin), 0);
    m_program.setUniformValue(m_matrixUniform, matrix);
    m_program.setUniformValue(m_screenUniform, m_width, m_height);
    m_openGLFunctions->glLineWidth(m_weight * m_retinaScale);
    GLfloat vertices[8];

    m_openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0,
                                             static_cast<const void *>(vertices));
    m_openGLFunctions->glEnableVertexAttribArray(0);

    float floor = -140.f;
    float alpha;

    historyRowData rowData;
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

    auto accumalte = [m_source = m_source, &value] (unsigned int i) {
        value += powf(m_source->module(i), 2);
    };
    auto collected = [&]
    (float start, float end, unsigned int count) {
        Q_UNUSED(count)

        value = 10 * log10f(value);

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
        rgb[0] = start;
        rgb[1] = end;
        rgb[2] = static_cast<float>(pointColor.redF());
        rgb[3] = static_cast<float>(pointColor.greenF());
        rgb[4] = static_cast<float>(pointColor.blueF());
        rgb[5] = alpha;
        rowData.push_back(rgb);

        value = 0;
    };
    iterate(m_pointsPerOctave, accumalte, collected);

    historyRow row;
    row.time = static_cast<int>(m_timer.restart());
    row.data = rowData;
    history.push_back(row);
    if (history.size() > 51) {
        history.pop_front();
    }

    float t(0), tStep(0);
    for (auto row = history.crbegin(); row != history.crend(); ++row) {

        const historyRowData *rowData = &(row->data);
        tStep = row->time / 1000.f;

        for (unsigned int i = 1; i < rowData->size(); ++i) {

            vertices[0] = (rowData->at(i - 1)[0] + rowData->at(i - 1)[1]) / 2;
            vertices[1] = t;
            vertices[2] = (rowData->at(i - 1)[0] + rowData->at(i - 1)[1]) / 2;
            vertices[3] = t + tStep;
            vertices[4] = (rowData->at(i)[0] + rowData->at(i)[1]) / 2;
            vertices[5] = t;
            vertices[6] = (rowData->at(i)[0] + rowData->at(i)[1]) / 2;
            vertices[7] = t + tStep;

            m_program.setUniformValue(
                m_colorLeftUniform,
                rowData->at(i - 1)[2],
                rowData->at(i - 1)[3],
                rowData->at(i - 1)[4],
                rowData->at(i - 1)[5]
            );
            m_program.setUniformValue(
                m_colorRightUniform,
                rowData->at(i)[2],
                rowData->at(i)[3],
                rowData->at(i)[4],
                rowData->at(i)[5]
            );

            if (i > 1) {
                m_program.setUniformValue(m_prePositionAttr,  (rowData->at(i - 1)[0] + rowData->at(i - 1)[1]) / 2,
                                          vertices[3], 0.f, 1.f);
                m_program.setUniformValue(m_postPositionAttr, (rowData->at(i  )[0] + rowData->at(i  )[1]) / 2,
                                          vertices[1], 0.f, 1.f);
                m_openGLFunctions->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
        t += tStep;
    }
    m_openGLFunctions->glDisableVertexAttribArray(0);
}
