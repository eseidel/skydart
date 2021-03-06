/*
 * Copyright (C) 2004, 2006, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2010 Torch Mobile (Beijing) Co. Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SKY_ENGINE_CORE_HTML_HTMLCANVASELEMENT_H_
#define SKY_ENGINE_CORE_HTML_HTMLCANVASELEMENT_H_

#include "base/cancelable_callback.h"
#include "sky/engine/core/dom/Document.h"
#include "sky/engine/core/html/HTMLElement.h"
#include "sky/engine/core/html/canvas/CanvasImageSource.h"
#include "sky/engine/platform/geometry/FloatRect.h"
#include "sky/engine/platform/geometry/IntSize.h"
#include "sky/engine/platform/graphics/Canvas2DLayerBridge.h"
#include "sky/engine/platform/graphics/GraphicsTypes.h"
#include "sky/engine/platform/graphics/ImageBufferClient.h"
#include "sky/engine/platform/heap/Handle.h"
#include "sky/engine/public/platform/WebThread.h"
#include "sky/engine/wtf/Forward.h"

#define CanvasDefaultInterpolationQuality InterpolationLow

namespace blink {

class AffineTransform;
class CanvasContextAttributes;
class CanvasRenderingContext;
class GraphicsContext;
class GraphicsContextStateSaver;
class HTMLCanvasElement;
class Image;
class ImageData;
class ImageBuffer;
class ImageBufferSurface;
class IntSize;

class CanvasObserver {
    DECLARE_EMPTY_VIRTUAL_DESTRUCTOR_WILL_BE_REMOVED(CanvasObserver);
public:
    virtual void canvasChanged(HTMLCanvasElement*, const FloatRect& changedRect) = 0;
    virtual void canvasResized(HTMLCanvasElement*) = 0;
#if !ENABLE(OILPAN)
    virtual void canvasDestroyed(HTMLCanvasElement*) = 0;
#endif
};

class HTMLCanvasElement final : public HTMLElement, public CanvasImageSource, public ImageBufferClient {
    DEFINE_WRAPPERTYPEINFO();
public:
    DECLARE_NODE_FACTORY(HTMLCanvasElement);
    virtual ~HTMLCanvasElement();

    void addObserver(CanvasObserver*);
    void removeObserver(CanvasObserver*);

    // Attributes and functions exposed to script
    int width() const { return size().width(); }
    int height() const { return size().height(); }

    const IntSize& size() const { return m_size; }

    void setWidth(int);
    void setHeight(int);
    void setAccelerationDisabled(bool accelerationDisabled) { m_accelerationDisabled = accelerationDisabled; }
    bool accelerationDisabled() const { return m_accelerationDisabled; }

    void setSize(const IntSize& newSize)
    {
        if (newSize == size())
            return;
        m_ignoreReset = true;
        setWidth(newSize.width());
        setHeight(newSize.height());
        m_ignoreReset = false;
        reset();
    }

    CanvasRenderingContext2D* getContext(const String&, CanvasContextAttributes* attributes = 0);

    static String toEncodingMimeType(const String& mimeType);
    String toDataURL(const String& mimeType, const double* quality, ExceptionState&) const;
    String toDataURL(const String& mimeType, ExceptionState& exceptionState) const { return toDataURL(mimeType, 0, exceptionState); }

    // Used for rendering
    void didDraw(const FloatRect&);
    void notifyObserversCanvasChanged(const FloatRect&);

    void paint(GraphicsContext*, const LayoutRect&);

    GraphicsContext* drawingContext() const;
    GraphicsContext* existingDrawingContext() const;

    CanvasRenderingContext* renderingContext() const { return m_context.get(); }

    void ensureUnacceleratedImageBuffer();
    ImageBuffer* buffer() const;
    Image* copiedImage() const;
    void clearCopiedImage();
    PassRefPtr<ImageData> getImageData() const;
    void makePresentationCopy();
    void clearPresentationCopy();

    AffineTransform baseTransform() const;

    bool is3D() const;

    bool hasImageBuffer() const { return m_imageBuffer; }
    bool hasValidImageBuffer() const;
    void discardImageBuffer();

    bool shouldAccelerate(const IntSize&) const;

    virtual const AtomicString imageSourceURL() const override;

    // CanvasImageSource implementation
    virtual PassRefPtr<Image> getSourceImageForCanvas(SourceImageMode, SourceImageStatus*) const override;
    virtual FloatSize sourceSize() const override;

    // ImageBufferClient implementation
    virtual void notifySurfaceInvalid() override;
    virtual bool isDirty() override { return !m_dirtyRect.isEmpty(); }
    virtual void didFinalizeFrame() override;

private:
    explicit HTMLCanvasElement(Document&);

    virtual void parseAttribute(const QualifiedName&, const AtomicString&) override;
    virtual RenderObject* createRenderer(RenderStyle*) override;

    void finalizeFrameMicrotask();

    void reset();

    PassOwnPtr<ImageBufferSurface> createImageBufferSurface(const IntSize& deviceSize, int* msaaSampleCount);
    void createImageBuffer();
    void createImageBufferInternal();
    void clearImageBuffer();

    void resetDirtyRect();

    void setSurfaceSize(const IntSize&);

    bool paintsIntoCanvasBuffer() const;

    void updateExternallyAllocatedMemory() const;

    String toDataURLInternal(const String& mimeType, const double* quality, bool isSaving = false) const;

    HashSet<RawPtr<CanvasObserver> > m_observers;

    IntSize m_size;

    OwnPtr<CanvasRenderingContext> m_context;

    bool m_ignoreReset;
    bool m_accelerationDisabled;
    // FIXME(sky): Do we need this in sky since we don't do paint invalidation?
    FloatRect m_dirtyRect;
    base::CancelableClosure m_finalizeFrameTask;

    mutable intptr_t m_externallyAllocatedMemory;

    // It prevents HTMLCanvasElement::buffer() from continuously re-attempting to allocate an imageBuffer
    // after the first attempt failed.
    mutable bool m_didFailToCreateImageBuffer;
    mutable bool m_didClearImageBuffer;
    OwnPtr<ImageBuffer> m_imageBuffer;
    mutable OwnPtr<GraphicsContextStateSaver> m_contextStateSaver;

    mutable RefPtr<Image> m_presentedImage;
    mutable RefPtr<Image> m_copiedImage; // FIXME: This is temporary for platforms that have to copy the image buffer to render.
};

} // namespace blink

#endif  // SKY_ENGINE_CORE_HTML_HTMLCANVASELEMENT_H_
