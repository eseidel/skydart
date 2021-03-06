// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/common/trace_controller_impl.h"

#include "base/trace_event/trace_event.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_impl.h"

namespace mojo {

TraceControllerImpl::TraceControllerImpl(
    InterfaceRequest<tracing::TraceController> request)
    : binding_(this, request.Pass()) {
}

TraceControllerImpl::~TraceControllerImpl() {
}

void TraceControllerImpl::StartTracing(
    const String& categories,
    tracing::TraceDataCollectorPtr collector) {
  DCHECK(!collector_.get());
  collector_ = collector.Pass();
  std::string categories_str = categories.To<std::string>();
  base::debug::TraceLog::GetInstance()->SetEnabled(
      base::debug::CategoryFilter(categories_str),
      base::debug::TraceLog::RECORDING_MODE,
      base::debug::TraceOptions(base::debug::RECORD_UNTIL_FULL));
}

void TraceControllerImpl::StopTracing() {
  base::debug::TraceLog::GetInstance()->SetDisabled();

  base::debug::TraceLog::GetInstance()->Flush(
      base::Bind(&TraceControllerImpl::SendChunk, base::Unretained(this)));
}

void TraceControllerImpl::SendChunk(
    const scoped_refptr<base::RefCountedString>& events_str,
    bool has_more_events) {
  collector_->DataCollected(mojo::String(events_str->data()));
  if (!has_more_events) {
    collector_.reset();
  }
}

}  // namespace mojo
