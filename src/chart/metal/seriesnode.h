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
#ifndef CHART_SERIESNODE_H
#define CHART_SERIESNODE_H

#include <QQuickItem>
#include <QSGTextureProvider>
#include <QSGSimpleTextureNode>

namespace chart {
class Source;
class Plot;

class SeriesNode : public QSGTextureProvider, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    SeriesNode(QQuickItem *item);
    virtual ~SeriesNode();

    int width() const;
    int height() const;

    QSGTexture *texture() const override final;

public slots:
    void synchronize();
    void render();

protected:
    Plot *plot() const;

    //! create new MTLRenderCommandEncoder
    void *commandEncoder();

    //! alloc and init MTLRenderPipelineDescriptor. Must be released
    void *allocPipelineStateDescriptor();

    //! encode empty MTLRenderCommandEncoder
    void clearRender();

    virtual void initRender() = 0;
    virtual void synchronizeSeries() = 0;
    virtual void renderSeries() = 0;

    QQuickItem *m_item;
    Source *m_source;

    //! MTLLibrary
    void *m_library;
    qreal m_devicePixelRatio;

    //! MTLRenderPipelineState
    void *m_clearPipeline;

    //! MTLBuffer
    void *m_colorBuffer;

    //! MTLBuffer
    void *m_widthBuffer;

    //! MTLBuffer
    void *m_sizeBuffer;

    //! MTLDevice
    void *m_device;

private:
    void init();
    bool m_initialized;

    //! MTLTexture
    void *m_texture;

    QQuickWindow *m_window;
    QSize m_size;
    float m_retinaScale;
};

} // namespace chart

#endif // CHART_SERIESNODE_H
