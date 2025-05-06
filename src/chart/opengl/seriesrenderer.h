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
#ifndef SERIESRENDERER_H
#define SERIESRENDERER_H

#include <QQuickFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>

#include "abstract/source.h"

namespace Chart {
class Plot;
class SeriesRenderer : public QQuickFramebufferObject::Renderer
{
public:
    explicit SeriesRenderer();
    virtual ~SeriesRenderer() override;
    virtual void init() {};//TODO: = 0;
    void render() override final;
    virtual void renderSeries() = 0;

    virtual void setWeight(unsigned int weight);

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *item) override;

    void setOnDelete(const std::function<void ()> &newOnDelete);

protected:
    float coherenceSpline(const bool &coherence, const float &threshold, const float data[4], const float &t) const;
    void addLinePoint(unsigned int &i, unsigned int &verticiesCount, const float &x, const float &y,
                      const float &c = 1);
    void addLineSegment(unsigned int &i, unsigned int &verticiesCount,
                        const float &fromX, const float &fromY,
                        const float &toX, const float &toY,
                        const float &fromC, const float &toC);
    void drawOpenGL2(unsigned int verticiesCount, GLenum mode = GL_LINES);

    Plot *plot() const;

    Shared::Source m_source;
    QQuickFramebufferObject *m_item = nullptr;
    QOpenGLShaderProgram m_program;
    QOpenGLFunctions *m_openGLFunctions;
    QOpenGLFunctions_3_3_Core *m_openGL33CoreFunctions = nullptr;
    GLfloat m_retinaScale;
    int m_colorUniform, m_positionAttribute;
    GLsizei m_width, m_height;
    float m_weight;
    bool m_renderActive;

    bool m_refreshBuffers;
    unsigned int m_vertexBufferId, m_vertexArrayId;
    std::vector<GLfloat> m_vertices;

    static const unsigned int MAX_LINE_SPLIT = 40;
    constexpr static const float MAX_LINE_SPLITF = static_cast<float>(MAX_LINE_SPLIT);
    static const unsigned int LINE_VERTEX_SIZE = 6; // x,y + r,g,b,a
    static const unsigned int VERTEX_PER_SEGMENT = 2;
    constexpr static const unsigned int PPO_BUFFER_MUL = 13 * VERTEX_PER_SEGMENT * LINE_VERTEX_SIZE * MAX_LINE_SPLITF;

private:
    std::mutex m_active;
    std::function<void()> m_onDelete = nullptr;
};
}
#endif // SERIESRENDERER_H
