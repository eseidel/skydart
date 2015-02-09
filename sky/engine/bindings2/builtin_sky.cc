// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/builtin_sky.h"

#include "sky/engine/bindings2/builtin.h"
#include "sky/engine/core/frame/LocalDOMWindow.h"
#include "sky/engine/core/script/core_dart_state.h"
#include "sky/engine/tonic/dart_error.h"

namespace blink {

BuiltinSky::BuiltinSky(CoreDartState* dart_state) {
  Builtin::SetNativeResolver(Builtin::kSkyLibrary);
  library_.Set(dart_state, Builtin::LoadAndCheckLibrary(Builtin::kSkyLibrary));
}

BuiltinSky::~BuiltinSky() {
}

void BuiltinSky::InstallWindow(CoreDartState* dart_state) {
  CHECK(!LogIfError(Dart_SetField(library_.value(),
    Dart_NewStringFromCString("window"),
    ToDart(dart_state->CurrentWindow()))));
  CHECK(!LogIfError(Dart_SetField(library_.value(),
    Dart_NewStringFromCString("document"),
    ToDart(dart_state->CurrentDocument()))));
}

Dart_Handle BuiltinSky::GetClassByName(const char* class_name) {
  Dart_Handle name_handle = Dart_NewStringFromCString(class_name);
  Dart_Handle class_handle = Dart_GetType(library_.value(), name_handle, 0, nullptr);
  DCHECK(!Dart_IsError(class_handle)) << class_name;
  return class_handle;
}

}  // namespace blink
