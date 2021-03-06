// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/external_application_listener.h"

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/sequenced_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_checker.h"
#include "base/tracked_objects.h"
#include "mojo/common/common_type_converters.h"
#include "mojo/edk/embedder/channel_init.h"
#include "mojo/public/cpp/bindings/error_handler.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "shell/domain_socket/net_errors.h"
#include "shell/domain_socket/socket_descriptor.h"
#include "shell/external_application_registrar.mojom.h"
#include "shell/incoming_connection_listener.h"
#include "url/gurl.h"

namespace mojo {
namespace shell {

namespace {
const char kDefaultListenSocketPath[] = "/var/run/mojo/system_socket";

class RegistrarImpl : public ExternalApplicationRegistrar, public ErrorHandler {
 public:
  RegistrarImpl(const ExternalApplicationListener::RegisterCallback& callback,
                SocketDescriptor incoming_socket,
                scoped_refptr<base::TaskRunner> io_runner)
      : register_callback_(callback),
        binding_(this, channel_init_.Init(incoming_socket, io_runner)) {
    binding_.set_error_handler(this);
  }

  ~RegistrarImpl() override {}

 private:
  void OnConnectionError() override { channel_init_.WillDestroySoon(); }

  void Register(
      const String& app_url,
      Array<String> args,
      const Callback<void(InterfaceRequest<Application>)>& callback) override {
    ApplicationPtr application;
    InterfaceRequest<Application> application_request = GetProxy(&application);
    register_callback_.Run(app_url.To<GURL>(),
                           args.To<std::vector<std::string>>(),
                           application.Pass());
    callback.Run(application_request.Pass());
  }

  embedder::ChannelInit channel_init_;
  const ExternalApplicationListener::RegisterCallback register_callback_;
  StrongBinding<ExternalApplicationRegistrar> binding_;

  DISALLOW_COPY_AND_ASSIGN(RegistrarImpl);
};

}  // namespace

// static
base::FilePath ExternalApplicationListener::ConstructDefaultSocketPath() {
  return base::FilePath(FILE_PATH_LITERAL(kDefaultListenSocketPath));
}

ExternalApplicationListener::ExternalApplicationListener(
    const scoped_refptr<base::SequencedTaskRunner>& shell_runner,
    const scoped_refptr<base::SequencedTaskRunner>& io_runner)
    : shell_runner_(shell_runner),
      io_runner_(io_runner),
      signal_on_listening_(true, false),
      weak_ptr_factory_(this) {
  DCHECK(shell_runner_.get() && shell_runner_->RunsTasksOnCurrentThread());
  DCHECK(io_runner_.get());
  listener_thread_checker_.DetachFromThread();  // Will attach in StartListener.
}

ExternalApplicationListener::~ExternalApplicationListener() {
  DCHECK(register_thread_checker_.CalledOnValidThread());
  weak_ptr_factory_.InvalidateWeakPtrs();

  // listener_ needs to be destroyed on io_runner_, and it has to die before
  // this object does, as it holds a pointer back to this instance.
  base::WaitableEvent stop_listening_event(true, false);
  io_runner_->PostTask(
      FROM_HERE, base::Bind(&ExternalApplicationListener::StopListening,
                            base::Unretained(this), &stop_listening_event));
  stop_listening_event.Wait();
}

void ExternalApplicationListener::ListenInBackground(
    const base::FilePath& listen_socket_path,
    const RegisterCallback& register_callback) {
  DCHECK(register_thread_checker_.CalledOnValidThread());
  ListenInBackgroundWithErrorCallback(listen_socket_path, register_callback,
                                      ErrorCallback());
}

void ExternalApplicationListener::ListenInBackgroundWithErrorCallback(
    const base::FilePath& listen_socket_path,
    const RegisterCallback& register_callback,
    const ErrorCallback& error_callback) {
  DCHECK(register_thread_checker_.CalledOnValidThread());
  register_callback_ = register_callback;
  error_callback_ = error_callback;

  io_runner_->PostTask(FROM_HERE,
                       base::Bind(&ExternalApplicationListener::StartListening,
                                  base::Unretained(this), listen_socket_path));
}

void ExternalApplicationListener::WaitForListening() {
  DCHECK(register_thread_checker_.CalledOnValidThread());
  signal_on_listening_.Wait();
}

void ExternalApplicationListener::StartListening(
    const base::FilePath& listen_socket_path) {
  CHECK_EQ(base::MessageLoop::current()->type(), base::MessageLoop::TYPE_IO);
  DCHECK(listener_thread_checker_.CalledOnValidThread());
  listener_.reset(new IncomingConnectionListener(listen_socket_path, this));
  listener_->StartListening();
}

void ExternalApplicationListener::StopListening(base::WaitableEvent* event) {
  DCHECK(listener_thread_checker_.CalledOnValidThread());
  listener_.reset();
  event->Signal();
}

void ExternalApplicationListener::OnListening(int rv) {
  DCHECK(listener_thread_checker_.CalledOnValidThread());
  signal_on_listening_.Signal();
  shell_runner_->PostTask(
      FROM_HERE,
      base::Bind(
          &ExternalApplicationListener::RunErrorCallbackIfListeningFailed,
          weak_ptr_factory_.GetWeakPtr(), rv));
}

void ExternalApplicationListener::OnConnection(SocketDescriptor incoming) {
  DCHECK(listener_thread_checker_.CalledOnValidThread());
  shell_runner_->PostTask(
      FROM_HERE, base::Bind(&ExternalApplicationListener::CreateRegistrar,
                            weak_ptr_factory_.GetWeakPtr(), incoming));
}

void ExternalApplicationListener::RunErrorCallbackIfListeningFailed(int rv) {
  DCHECK(register_thread_checker_.CalledOnValidThread());
  if (rv != net::OK && !error_callback_.is_null())
    error_callback_.Run(rv);
}

void ExternalApplicationListener::CreateRegistrar(
    SocketDescriptor incoming_socket) {
  DCHECK(register_thread_checker_.CalledOnValidThread());

  DVLOG(1) << "Accepted incoming connection";
  // Passes ownership of incoming_socket to registrar.
  new RegistrarImpl(register_callback_, incoming_socket, io_runner_);
}

}  // namespace shell
}  // namespace mojo
