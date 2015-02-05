// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_CORE_SCRIPT_CORE_DART_STATE_H_
#define SKY_ENGINE_CORE_SCRIPT_CORE_DART_STATE_H_

#include "dart/runtime/include/dart_api.h"
#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/core/dom/Document.h"
#include "sky/engine/wtf/RefPtr.h"

namespace blink {
class LocalFrame;
class LocalDOMWindow;

class CoreDartState : public DartState {
 public:
  explicit CoreDartState(Document* document);
  ~CoreDartState() override;

  static CoreDartState* Current();

  static Document* CurrentDocument();
  static LocalFrame* CurrentFrame();
  static LocalDOMWindow* CurrentWindow();

 private:
  RefPtr<Document> document_;
};

}

#endif // SKY_ENGINE_CORE_SCRIPT_CORE_DART_STATE_H_
