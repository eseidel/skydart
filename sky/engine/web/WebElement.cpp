/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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
#include "sky/engine/public/web/WebElement.h"

#include "sky/engine/bindings2/exception_state.h"
#include "sky/engine/core/dom/Element.h"
#include "sky/engine/core/dom/custom/CustomElementProcessingStack.h"
#include "sky/engine/core/dom/shadow/ShadowRoot.h"
#include "sky/engine/core/rendering/RenderBoxModelObject.h"
#include "sky/engine/core/rendering/RenderObject.h"
#include "sky/engine/public/platform/WebRect.h"
#include "sky/engine/public/web/WebDocument.h"
#include "sky/engine/wtf/PassRefPtr.h"

namespace blink {

WebString WebElement::tagName() const
{
    return constUnwrap<Element>()->tagName();
}

bool WebElement::hasAttribute(const WebString& attrName) const
{
    return constUnwrap<Element>()->hasAttribute(attrName);
}

void WebElement::removeAttribute(const WebString& attrName)
{
    // TODO: Custom element callbacks need to be called on WebKit API methods that
    // mutate the DOM in any way.
    CustomElementProcessingStack::CallbackDeliveryScope deliverCustomElementCallbacks;
    unwrap<Element>()->removeAttribute(attrName);
}

WebString WebElement::getAttribute(const WebString& attrName) const
{
    return constUnwrap<Element>()->getAttribute(attrName);
}

bool WebElement::setAttribute(const WebString& attrName, const WebString& attrValue)
{
    // TODO: Custom element callbacks need to be called on WebKit API methods that
    // mutate the DOM in any way.
    CustomElementProcessingStack::CallbackDeliveryScope deliverCustomElementCallbacks;
    TrackExceptionState exceptionState;
    unwrap<Element>()->setAttribute(attrName, attrValue, exceptionState);
    return !exceptionState.had_exception();
}

unsigned WebElement::attributeCount() const
{
    if (!constUnwrap<Element>()->hasAttributes())
        return 0;
    return constUnwrap<Element>()->attributes().size();
}

WebNode WebElement::shadowRoot() const
{
    ShadowRoot* shadowRoot = constUnwrap<Element>()->shadowRoot();
    if (!shadowRoot)
        return WebNode();
    return WebNode(shadowRoot->toNode());
}

WebString WebElement::attributeLocalName(unsigned index) const
{
    if (index >= attributeCount())
        return WebString();
    return constUnwrap<Element>()->attributes().at(index).localName();
}

WebString WebElement::attributeValue(unsigned index) const
{
    if (index >= attributeCount())
        return WebString();
    return constUnwrap<Element>()->attributes().at(index).value();
}

WebString WebElement::computeInheritedLanguage() const
{
    return WebString(constUnwrap<Element>()->computeInheritedLanguage());
}

void WebElement::requestFullScreen()
{
}

WebElement::WebElement(const PassRefPtr<Element>& elem)
    : WebNode(elem)
{
}

WebElement& WebElement::operator=(const PassRefPtr<Element>& elem)
{
    m_private = elem;
    return *this;
}

WebElement::operator PassRefPtr<Element>() const
{
    return toElement(m_private.get());
}

} // namespace blink
