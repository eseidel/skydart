// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_callback.h"

#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_state.h"

namespace blink {

DartCallback::DartCallback(DartState* dart_state,
                           Dart_Handle callback,
                           Dart_Handle& exception)
    : callback_(dart_state, callback) {
  if (!Dart_IsClosure(callback)) {
    exception = Dart_NewStringFromCString("Callback must be a function");
    callback_.Clear();
  }
}

DartCallback::~DartCallback() {
}

bool DartCallback::IsIsolateAlive() const {
  return !!callback_.dart_state();
}

Dart_Isolate DartCallback::GetIsolate() const {
  return callback_.dart_state()->isolate();
}

bool DartCallback::handleEvent(int argc, Dart_Handle* argv) {
  LogIfError(Dart_InvokeClosure(callback_.value(), argc, argv));
  return true;
}

}  // namespace blink
