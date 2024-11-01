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

namespace Chart {

#define id_cast(T, t) static_cast<id<T>>(t)

SeriesNode::SeriesNode(QQuickItem *item) : m_item(static_cast<SeriesItem *>(item)),
    m_library(nullptr), m_devicePixelRatio(1),
    m_clearPipeline(nullptr),
    m_colorBuffer(nullptr), m_widthBuffer(nullptr), m_sizeBuffer(nullptr),
    m_device(nullptr), m_commandQueue(nullptr), m_commandBuffer(nullptr), m_active(),
    m_initialized(false), m_texture(nullptr),
    m_glTexture(nullptr), m_buffer(nullptr),
    m_window(item->window()),
    m_size(), m_renderActive(false), m_readyRender(true)

{
    m_source = m_item->source();
    m_retinaScale = m_window->devicePixelRatio();

    setTextureCoordinatesTransform(QSGSimpleTextureNode::NoTransform);
    setFiltering(QSGTexture::Linear);
    connect(m_window, &QQuickWindow::screenChanged, this, [this]() {
        if (m_window->effectiveDevicePixelRatio() != m_devicePixelRatio)
            m_item->update();
    });
    connect(m_item, &SeriesItem::updated, this, [this]() {
        m_readyRender.store(true);
    });
    connect(m_window, &QQuickWindow::afterSynchronizing, this, &SeriesNode::synchronize, Qt::DirectConnection);
    connect(m_window, &QQuickWindow::beforeRendering,    this, [this]() {
        if (m_readyRender.load()) {
            m_readyRender.store(false);
            render();
        }
    },      Qt::DirectConnection);

    connect(m_item, &SeriesItem::preSourceDeleted, this, [this]() {
        m_active.lock();
        m_source.reset();
        m_active.unlock();
    }, Qt::DirectConnection);
}

SeriesNode::~SeriesNode()
{
    if (m_glTexture) {
        m_glTexture->deleteLater();
    }

    [id_cast(MTLBuffer, m_sizeBuffer) release];
    [id_cast(MTLBuffer, m_widthBuffer) release];
    [id_cast(MTLBuffer, m_colorBuffer) release];
    [id_cast(MTLRenderPipelineState, m_clearPipeline) release];
    [id_cast(MTLLibrary, m_library) release];

    delete texture();
    [id_cast(MTLTexture, m_texture) release];

    [id_cast(MTLTexture, m_commandQueue) release];
    [id_cast(MTLDevice, m_device) release];
}

QSGRendererInterface::GraphicsApi SeriesNode::m_backend = QSGRendererInterface::MetalRhi;

QSGRendererInterface::GraphicsApi SeriesNode::chooseRhi()
{
#ifdef FORCE_OPENGL_RHI
    m_backend = QSGRendererInterface::OpenGLRhi;
    qDebug() << "force OpenGL chosen";

    return m_backend;
#endif

    auto device = MTLCreateSystemDefaultDevice();
    if (@available(macOS 10.15, ios 13.0, *)) {
        if ([device supportsFamily:MTLGPUFamilyApple3]) {
            m_backend = QSGRendererInterface::MetalRhi;
            qDebug() << "Metal chosen";
            [device release];
            return m_backend;
        }

        if ([device supportsFamily:MTLGPUFamilyMac2]) {
            m_backend = QSGRendererInterface::MetalRhi;
            qDebug() << "Metal chosen";
            [device release];
            return m_backend;
        }
    }


    m_backend = QSGRendererInterface::OpenGLRhi;
    qDebug() << "OpenGL chosen";
    [device release];
    return m_backend;
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
    if (plot() && m_source) {
        m_renderActive = plot()->isSelected(m_source->uuid());
        m_weight = m_retinaScale * plot()->palette().lineWidth(m_item->highlighted());
        synchronizeSeries();
    }
}

void SeriesNode::init()
{
    delete texture();
    if (m_texture) {
        [id_cast(MTLTexture, m_texture) release];
        m_texture = nullptr;
    }

    if (m_device) {
        [id_cast(MTLDevice, m_device) release];
        m_device = nullptr;
    }
    m_device = MTLCreateSystemDefaultDevice();
    auto device = id_cast(MTLDevice, m_device);
    if (!device) {
        qCritical() << "no metal device";
        return ;
    }

    if (m_commandQueue) {
        [id_cast(MTLTexture, m_commandQueue) release];
        m_commandQueue = nullptr;
    }
    m_commandQueue = [id_cast(MTLDevice, m_device) newCommandQueue];
    if (!m_commandQueue) {
        qCritical() << "no commandQueue";
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

    if (m_backend == QSGRendererInterface::OpenGLRhi) {
        m_buffer = [
                       id_cast(MTLDevice, m_device)
                       newBufferWithLength: width() * height() * 4
                       options: MTLResourceStorageModeShared
                   ];

        m_image = QImage(width(), height(), QImage::Format_RGBA8888);
        m_glTexture = m_window->createTextureFromImage(m_image, QQuickWindow::TextureHasAlphaChannel);
        setTexture(m_glTexture);
    }

    if (m_backend == QSGRendererInterface::MetalRhi) {
        QSGTexture *wrapper = m_window->createTextureFromNativeObject(
                                  QQuickWindow::NativeObjectTexture,
                                  &m_texture,
                                  0,
                                  m_size, {QQuickWindow::TextureHasAlphaChannel}
                              );
        setTexture(wrapper);
    }

    if (m_initialized) {
        return ;
    }

    NSError *libraryError = NULL;
    NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"lib" ofType:@"metallib"];
    if (!libraryFile) {
        qDebug() << "library file not found";
        plot()->setRendererError("Metal library file not found");
        return;
    }
    m_library = [id_cast(MTLDevice, m_device) newLibraryWithFile:libraryFile error:&libraryError];
    if (!m_library) {
        qDebug() << @"Library error: %@" << [libraryError localizedDescription] << [libraryError localizedFailureReason];
        plot()->setRendererError(
            "Library error: " +
            QString::fromNSString([libraryError localizedDescription]) + " " +
            QString::fromNSString([libraryError localizedFailureReason]));
        return;
    }

    auto pipelineStateDescriptor = static_cast<MTLRenderPipelineDescriptor *>(allocPipelineStateDescriptor());
    pipelineStateDescriptor.vertexFunction = [id_cast(MTLLibrary, m_library) newFunctionWithName:@"clearVertex" ];

    NSError *error = nil;
    m_clearPipeline = [
                          id_cast(MTLDevice, m_device)
                          newRenderPipelineStateWithDescriptor: pipelineStateDescriptor
                          error: &error];
    if (!m_clearPipeline) {
        const QString msg = QString::fromNSString(error.localizedDescription);
        plot()->setRendererError("Failed to create render pipeline state: " + msg);
        qDebug() << "Failed to create render pipeline state: %s", qPrintable(msg);
        return;
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
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
    renderPassDescriptor.colorAttachments[0].texture = id_cast(MTLTexture, m_texture);

    auto commandBuffer = [id_cast(MTLCommandQueue, m_commandQueue) commandBuffer];
    commandBuffer.label = @"SeriesCommandBuffer";

    Q_ASSERT(commandBuffer);
    auto encoder = [id_cast(MTLCommandBuffer, commandBuffer) renderCommandEncoderWithDescriptor: renderPassDescriptor];
    m_commandBuffer = commandBuffer;

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

    if (!m_source || !m_renderActive || !m_source->active()) {
        clearRender();
    } else {
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

    if (!m_commandBuffer) {
        return;
    }

    switch (m_backend) {
    case QSGRendererInterface::MetalRhi:
        [id_cast(MTLCommandBuffer, m_commandBuffer) commit];
        break;
    case QSGRendererInterface::OpenGLRhi: {
        auto blitEncoder = [id_cast(MTLCommandBuffer, m_commandBuffer) blitCommandEncoder];

        [blitEncoder copyFromTexture: id_cast(MTLTexture, m_texture)
                     sourceSlice: 0
                     sourceLevel:0
                     sourceOrigin:MTLOriginMake(0, 0, 0)
                     sourceSize:MTLSizeMake(width(), height(), 1)
                     toBuffer:id_cast(MTLBuffer, m_buffer)
                     destinationOffset:0
                     destinationBytesPerRow:width() * 4
                     destinationBytesPerImage:width() * height() * 4
                     //options:(MTLBlitOption)options;
                    ];
        [blitEncoder endEncoding];

        [id_cast(MTLCommandBuffer, m_commandBuffer) commit];
        [id_cast(MTLCommandBuffer, m_commandBuffer) waitUntilCompleted];

        void *buffer_ptr = [id_cast(MTLBuffer, m_buffer) contents];
        if (buffer_ptr) {
            m_byteArray.resize(width() * height() * 4);
            std::memcpy(m_byteArray.data(), buffer_ptr, m_byteArray.size());

            char *data_ptr = m_byteArray.data();
            for (int y = 0; y < m_image.height(); y++) {
                memcpy(m_image.scanLine(y), data_ptr + y * m_image.bytesPerLine(), m_image.bytesPerLine());
            }


            if (m_glTexture) delete (m_glTexture);
            m_glTexture = m_window->createTextureFromImage(m_image);
            setTexture(m_glTexture);
        }
    }
    break;
    default:
        Q_UNREACHABLE();
    }

    m_commandBuffer = nullptr;
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

    auto commandBuffer = [id_cast(MTLCommandQueue, m_commandQueue) commandBuffer];
    commandBuffer.label = @"ClearSeriesCommandBuffer";
    Q_ASSERT(commandBuffer);
    id<MTLRenderCommandEncoder> encoder = [
                                              id_cast(MTLCommandBuffer, commandBuffer)
                                              renderCommandEncoderWithDescriptor: renderPassDescriptor];
    m_commandBuffer = commandBuffer;

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
