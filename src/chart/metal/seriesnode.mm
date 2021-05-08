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
#include "seriesnode.h"
#include "seriesitem.h"
#include "../plot.h"

#include <QScreen>
#include <QQuickWindow>
#include <Metal/Metal.h>

namespace chart {

#define id_cast(T, t) static_cast<id<T>>(t)

SeriesNode::SeriesNode(QQuickItem *item) : m_item(static_cast<SeriesItem *>(item)),
    m_library(nullptr), m_devicePixelRatio(1),
    m_clearPipeline(nullptr),
    m_colorBuffer(nullptr), m_widthBuffer(nullptr), m_sizeBuffer(nullptr),
    m_device(nullptr), m_initialized(false), m_texture(nullptr), m_window(item->window()),
    m_size(), m_active(), m_renderActive(false)

{
    m_source = m_item->source();
    m_retinaScale = m_window->devicePixelRatio();

    setTextureCoordinatesTransform(QSGSimpleTextureNode::NoTransform);
    setFiltering(QSGTexture::Linear);

    connect(m_window, &QQuickWindow::screenChanged, this, [this]() {
        if (m_window->effectiveDevicePixelRatio() != m_devicePixelRatio)
            m_item->update();
    });
    connect(m_window, &QQuickWindow::afterSynchronizing, this, &SeriesNode::synchronize, Qt::DirectConnection);
    connect(m_window, &QQuickWindow::beforeRendering,    this, &SeriesNode::render,      Qt::DirectConnection);
    connect(m_item, &SeriesItem::preSourceDeleted, this, [this]() {
        m_active.lock();
        m_source = nullptr;
        m_active.unlock();
    }, Qt::DirectConnection);
}

SeriesNode::~SeriesNode()
{
    [id_cast(MTLBuffer, m_sizeBuffer) release];
    [id_cast(MTLBuffer, m_widthBuffer) release];
    [id_cast(MTLBuffer, m_colorBuffer) release];

    delete texture();
    [id_cast(MTLTexture, m_texture) release];
}

void SeriesNode::synchronize()
{
    m_devicePixelRatio = m_window->effectiveDevicePixelRatio();
    QSize itemSize = QSize(m_item->width(), m_item->height());
    const QSize newSize = itemSize * m_devicePixelRatio;
    if (newSize != m_size || !texture()) {
        m_size = newSize;
        setRect(0, 0, m_item->width(), m_item->height());
        init();
    }
    m_renderActive = !plot()->filter() || plot()->filter() == m_source;
    m_weight = m_retinaScale * plot()->palette().lineWidth(m_item->highlighted());
    synchronizeSeries();
}

void SeriesNode::init()
{
    delete texture();
    if (m_texture) {
        [id_cast(MTLTexture, m_texture) release];
        m_texture = nullptr;
    }

    QSGRendererInterface *renderer = m_window->rendererInterface();
    m_device = renderer->getResource(m_window, QSGRendererInterface::DeviceResource);
    auto device = id_cast(MTLDevice, m_device);
    if (!device) {
        qCritical() << "no metal device";
        return ;
    }

    MTLTextureDescriptor *descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.textureType = MTLTextureType2D;
    descriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
    descriptor.width = width();
    descriptor.height = height();
    descriptor.mipmapLevelCount = 1;
    descriptor.resourceOptions = MTLResourceStorageModePrivate;
    descriptor.storageMode = MTLStorageModePrivate;
    descriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    m_texture = [id_cast(MTLDevice, m_device) newTextureWithDescriptor: descriptor];
    [descriptor release];

    QSGTexture *wrapper = m_window->createTextureFromNativeObject(
                              QQuickWindow::NativeObjectTexture,
                              &m_texture,
                              0,
                              m_size, {QQuickWindow::TextureHasAlphaChannel}
                          );

    setTexture(wrapper);

    if (m_initialized) {
        return ;
    }

    NSError *libraryError = NULL;
    NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"lib" ofType:@"metallib"];
    if (!libraryFile) {
        qDebug() << "library file not found";
        return;
    }
    m_library = [id_cast(MTLDevice, m_device) newLibraryWithFile:libraryFile error:&libraryError];
    if (!m_library) {
        qDebug() << @"Library error: %@" << [libraryError localizedDescription];
    }

    auto pipelineStateDescriptor = static_cast<MTLRenderPipelineDescriptor *>(allocPipelineStateDescriptor());
    pipelineStateDescriptor.vertexFunction = [id_cast(MTLLibrary, m_library) newFunctionWithName:@"clearVertex" ];

    NSError *error = nil;
    m_clearPipeline = [id_cast(MTLDevice,
                               m_device) newRenderPipelineStateWithDescriptor: pipelineStateDescriptor error: &error];
    if (!m_clearPipeline) {
        const QString msg = QString::fromNSString(error.localizedDescription);
        qFatal("Failed to create render pipeline state: %s", qPrintable(msg));
    }
    [pipelineStateDescriptor release];

    m_colorBuffer = [
                        id_cast(MTLDevice, m_device)
                        newBufferWithLength: 4 * sizeof(float)
                        options: MTLResourceStorageModeShared
                    ];

    m_widthBuffer = [
                        id_cast(MTLDevice, m_device)
                        newBufferWithLength: sizeof(float)
                        options: MTLResourceStorageModeShared
                    ];

    m_sizeBuffer = [
                       id_cast(MTLDevice, m_device)
                       newBufferWithLength: 2 * sizeof(float)
                       options: MTLResourceStorageModeShared
                   ];

    initRender();
    m_initialized = true;
}

QSGTexture *SeriesNode::texture() const
{
    return QSGSimpleTextureNode::texture();
}

Plot *SeriesNode::plot() const
{
    return static_cast<Plot *>(m_item->parent());
}

void *SeriesNode::commandEncoder()
{
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    MTLClearColor c = MTLClearColorMake(0, 0, 0, 0);
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = c;
    renderPassDescriptor.colorAttachments[0].texture = id_cast(MTLTexture, m_texture);

    QSGRendererInterface *rendererInterface = m_window->rendererInterface();
    auto commandBuffer = rendererInterface->getResource(m_window, QSGRendererInterface::CommandListResource);
    Q_ASSERT(commandBuffer);

    auto encoder = [id_cast(MTLCommandBuffer, commandBuffer) renderCommandEncoderWithDescriptor: renderPassDescriptor];

    MTLViewport vp;
    vp.originX = 0;
    vp.originY = 0;
    vp.width = width();
    vp.height = height();
    vp.znear = 0;
    vp.zfar = 1;
    [encoder setViewport: vp];

    return encoder;
}

void *SeriesNode::allocPipelineStateDescriptor()
{
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
    pipelineStateDescriptor.colorAttachments[0].blendingEnabled = true;
    pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    return pipelineStateDescriptor;
}

void SeriesNode::render()
{
    std::lock_guard guard(m_active);
    if (!m_initialized)
        return;

    if (!m_source || !m_renderActive) {
        clearRender();
        return;
    }

    if (!m_source->active()) {
        clearRender();
        return;
    }

    auto color = m_source->color();
    float colorF[4];
    colorF[0] = color.redF();
    colorF[1] = color.greenF();
    colorF[2] = color.blueF();
    colorF[3] = color.alphaF();
    void *color_ptr = [id_cast(MTLBuffer, m_colorBuffer) contents];
    if (color_ptr) {
        memcpy(color_ptr, colorF, sizeof(colorF));
    }

    void *width_ptr = [id_cast(MTLBuffer, m_widthBuffer) contents];
    if (width_ptr) {
        memcpy(width_ptr, &m_weight, sizeof(float));
    }

    float *size_ptr = static_cast<float *>([id_cast(MTLBuffer, m_sizeBuffer) contents]);
    if (size_ptr) {
        size_ptr[0] = width();
        size_ptr[1] = height();
    }

    m_source->lock();
    renderSeries();
    m_source->unlock();
}

int SeriesNode::width() const
{
    return m_size.width();
}

int SeriesNode::height() const
{
    return m_size.height();
}

void SeriesNode::clearRender()
{
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    MTLClearColor c = MTLClearColorMake(0, 0, 0, 0);
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = c;
    renderPassDescriptor.colorAttachments[0].texture = id_cast(MTLTexture, m_texture);

    QSGRendererInterface *rendererInterface = m_window->rendererInterface();
    auto commandBuffer = rendererInterface->getResource(m_window, QSGRendererInterface::CommandListResource);
    Q_ASSERT(commandBuffer);
    id<MTLRenderCommandEncoder> encoder = [
                                              id_cast(MTLCommandBuffer, commandBuffer)
                                              renderCommandEncoderWithDescriptor: renderPassDescriptor];

    MTLViewport vp;
    vp.originX = 0;
    vp.originY = 0;
    vp.width = width();
    vp.height = height();
    vp.znear = 0;
    vp.zfar = 1;
    [encoder setViewport: vp];
    [encoder setRenderPipelineState: id_cast(MTLRenderPipelineState, m_clearPipeline)];
    [encoder endEncoding];
}


} // namespace chart
