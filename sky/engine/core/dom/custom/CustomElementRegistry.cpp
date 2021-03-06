/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include "sky/engine/config.h"
#include "sky/engine/core/dom/custom/CustomElementRegistry.h"

#include "sky/engine/core/dom/DocumentLifecycleObserver.h"
#include "sky/engine/core/dom/custom/CustomElementException.h"
#include "sky/engine/core/dom/custom/CustomElementRegistrationContext.h"

namespace blink {

class RegistrationContextObserver : public DocumentLifecycleObserver {
public:
    explicit RegistrationContextObserver(Document* document)
        : DocumentLifecycleObserver(document)
        , m_wentAway(!document)
    {
    }

    bool registrationContextWentAway() { return m_wentAway; }

private:
#if ENABLE(OILPAN)
    // In oilpan we don't have the disposed phase for context lifecycle observer.
    virtual void documentWasDetached() override { m_wentAway = true; }
#else
    virtual void documentWasDisposed() override { m_wentAway = true; }
#endif

    bool m_wentAway;
};

CustomElementDefinition* CustomElementRegistry::registerElement(Document* document, CustomElementConstructorBuilder* constructorBuilder, const AtomicString& userSuppliedName, ExceptionState& exceptionState)
{
    // TODO(dart): Figure out how to register a custom element.
    return 0;
}

CustomElementDefinition* CustomElementRegistry::find(const AtomicString& localName) const
{
    return m_definitions.get(localName);
}

} // namespace blink
