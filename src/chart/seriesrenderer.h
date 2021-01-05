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
#ifndef SERIESRENDERER_H
#define SERIESRENDERER_H

#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLFunctions>

#include "source.h"

namespace Fftchart {

class SeriesRenderer : public QQuickFramebufferObject::Renderer
{
public:
    explicit SeriesRenderer();
    virtual ~SeriesRenderer() override = default;
    void render() override final;
    virtual void renderSeries() = 0;
    virtual void setWeight(unsigned int weight);

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *item) override;

protected:
    Source *m_source = nullptr;
    QQuickFramebufferObject *m_item = nullptr;
    QOpenGLShaderProgram m_program;
    QOpenGLFunctions *openGLFunctions = nullptr;
    GLfloat m_retinaScale;
    int m_colorUniform;
    GLsizei m_width, m_height;
    float m_weight;
};
}
#endif // SERIESRENDERER_H
