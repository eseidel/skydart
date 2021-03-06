// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/icu/icu.h"

#include "mojo/public/cpp/application/application_impl.h"
#include "services/icu_data/icu_data.mojom.h"
#include "third_party/icu/source/common/unicode/putil.h"
#include "third_party/icu/source/common/unicode/udata.h"

#if !defined(OS_ANDROID)
#include "base/i18n/icu_util.h"
#endif

namespace mojo {
namespace icu {
namespace {

// These values are for Android arm32.
const char kDataHash[] = "3ef55e413a9d429e404e10fe8ec566009d3889f3";
const int64_t kDataSize = 6037440;

class Callback {
 public:
  void Run(mojo::ScopedSharedBufferHandle handle) const {
    void* ptr = nullptr;
    mojo::MapBuffer(handle.get(), 0, kDataSize, &ptr,
                    MOJO_MAP_BUFFER_FLAG_NONE);
    UErrorCode err = U_ZERO_ERROR;
    udata_setCommonData(ptr, &err);
    // Leak the handle because we never unmap the buffer.
    (void)handle.release();
  };
};
}

void Initialize(ApplicationImpl* app) {
#if !defined(OS_ANDROID)
  // On desktop platforms, the icu data table is stored in a file on disk, which
  // can be loaded using base.
  base::i18n::InitializeICU();
  return;
#endif

  icu_data::ICUDataPtr icu_data;
  app->ConnectToService("mojo:icu_data", &icu_data);
  icu_data->Map(kDataHash, Callback());
  icu_data.WaitForIncomingMethodCall();
}

}  // namespace icu
}  // namespace mojo
