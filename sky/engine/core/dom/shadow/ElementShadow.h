/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
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

#ifndef SKY_ENGINE_CORE_DOM_SHADOW_ELEMENTSHADOW_H_
#define SKY_ENGINE_CORE_DOM_SHADOW_ELEMENTSHADOW_H_

#include "sky/engine/core/dom/shadow/InsertionPoint.h"
#include "sky/engine/core/dom/shadow/SelectRuleFeatureSet.h"
#include "sky/engine/core/dom/shadow/ShadowRoot.h"
#include "sky/engine/platform/heap/Handle.h"
#include "sky/engine/wtf/HashMap.h"
#include "sky/engine/wtf/Noncopyable.h"
#include "sky/engine/wtf/PassOwnPtr.h"

namespace blink {

class ElementShadow final {
    WTF_MAKE_NONCOPYABLE(ElementShadow);
    WTF_MAKE_FAST_ALLOCATED;
public:
    static PassOwnPtr<ElementShadow> create();
    ~ElementShadow();

    Element* host() const;
    ShadowRoot* shadowRoot() const { return m_shadowRoot; }
    ElementShadow* containingShadow() const;

    ShadowRoot& addShadowRoot(Element& shadowHost);

    bool hasSameStyles(const ElementShadow*) const;

    void attach(const Node::AttachContext&);
    void detach(const Node::AttachContext&);

    void willAffectSelector();
    const SelectRuleFeatureSet& ensureSelectFeatureSet();

    void distributeIfNeeded();
    void setNeedsDistributionRecalc();

    const InsertionPoint* finalDestinationInsertionPointFor(const Node*) const;
    const DestinationInsertionPoints* destinationInsertionPointsFor(const Node*) const;

    void didDistributeNode(const Node*, InsertionPoint*);

private:
    ElementShadow();

    void removeDetachedShadowRoot();

    void distribute();
    void clearDistribution();

    void collectSelectFeatureSetFrom(ShadowRoot&);
    void distributeNodeChildrenTo(InsertionPoint*, ContainerNode*);

    bool needsSelectFeatureSet() const { return m_needsSelectFeatureSet; }
    void setNeedsSelectFeatureSet() { m_needsSelectFeatureSet = true; }

    typedef HashMap<RawPtr<const Node>, DestinationInsertionPoints> NodeToDestinationInsertionPoints;
    NodeToDestinationInsertionPoints m_nodeToInsertionPoints;

    SelectRuleFeatureSet m_selectFeatures;
    ShadowRoot* m_shadowRoot;
    bool m_needsDistributionRecalc;
    bool m_needsSelectFeatureSet;
};

inline Element* ElementShadow::host() const
{
    ASSERT(m_shadowRoot);
    return m_shadowRoot->host();
}

inline ElementShadow* ElementShadow::containingShadow() const
{
    if (ShadowRoot* parentRoot = host()->containingShadowRoot())
        return parentRoot->owner();
    return 0;
}

inline void ElementShadow::distributeIfNeeded()
{
    if (m_needsDistributionRecalc)
        distribute();
    m_needsDistributionRecalc = false;
}

} // namespace

#endif  // SKY_ENGINE_CORE_DOM_SHADOW_ELEMENTSHADOW_H_
