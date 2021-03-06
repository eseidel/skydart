/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SKY_ENGINE_CORE_HTML_CANVAS_CANVAS2DCONTEXTATTRIBUTES_H_
#define SKY_ENGINE_CORE_HTML_CANVAS_CANVAS2DCONTEXTATTRIBUTES_H_

#include "sky/engine/tonic/dart_wrappable.h"
#include "sky/engine/core/html/canvas/CanvasContextAttributes.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace blink {

enum Canvas2DContextStorage {
    PersistentStorage,
    DiscardableStorage
};

class Canvas2DContextAttributes : public CanvasContextAttributes, public DartWrappable {
    DECLARE_EMPTY_VIRTUAL_DESTRUCTOR_WILL_BE_REMOVED(Canvas2DContextAttributes);
    DEFINE_WRAPPERTYPEINFO();
public:
    // Create a new attributes object
    static PassRefPtr<Canvas2DContextAttributes> create();

    String storage() const;
    void setStorage(const String&);
    Canvas2DContextStorage parsedStorage() const;

protected:
    Canvas2DContextAttributes();

    Canvas2DContextStorage m_storage;
};

} // namespace blink

#endif  // SKY_ENGINE_CORE_HTML_CANVAS_CANVAS2DCONTEXTATTRIBUTES_H_
