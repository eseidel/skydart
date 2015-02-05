// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_ISOLATE_SCOPE_H_
#define SKY_ENGINE_BINDINGS2_DART_ISOLATE_SCOPE_H_

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/wtf/Noncopyable.h"

namespace blink {

// DartIsolateScope is a helper class for entering and exiting a given isolate.
class DartIsolateScope {
  WTF_MAKE_NONCOPYABLE(DartIsolateScope);

 public:
  explicit DartIsolateScope(Dart_Isolate isolate);
  ~DartIsolateScope();

 private:
  Dart_Isolate isolate_;
  Dart_Isolate previous_;
};
}

#endif  // SKY_ENGINE_BINDINGS2_DART_ISOLATE_SCOPE_H_
