// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_APP_CHILD_PROCESS_HOST_H_
#define SHELL_APP_CHILD_PROCESS_HOST_H_

#include "base/macros.h"
#include "mojo/edk/embedder/channel_info_forward.h"
#include "shell/app_child_process.mojom.h"
#include "shell/child_process_host.h"

namespace mojo {
namespace shell {

// A subclass of |ChildProcessHost| to host a |TYPE_APP| child process, which
// runs a single app (loaded from the file system).
//
// Note: After |Start()|, |StartApp| must be called and this object must
// remained alive until the |on_app_complete| callback is called.
class AppChildProcessHost : public ChildProcessHost,
                            public ChildProcessHost::Delegate {
 public:
  explicit AppChildProcessHost(Context* context);
  ~AppChildProcessHost() override;

  // See |AppChildController::StartApp|.
  void StartApp(const String& app_path,
                bool clean_app_path,
                InterfaceRequest<Application> application_request,
                const AppChildController::StartAppCallback& on_app_complete);

 private:
  // |ChildProcessHost::Delegate| methods:
  void WillStart() override;
  void DidStart(bool success) override;

  // Callback for |embedder::CreateChannel()|.
  void DidCreateChannel(embedder::ChannelInfo* channel_info);

  void AppCompleted(int32_t result);

  AppChildControllerPtr controller_;
  embedder::ChannelInfo* channel_info_;
  AppChildController::StartAppCallback on_app_complete_;

  DISALLOW_COPY_AND_ASSIGN(AppChildProcessHost);
};

}  // namespace shell
}  // namespace mojo

#endif  // SHELL_APP_CHILD_PROCESS_HOST_H_
