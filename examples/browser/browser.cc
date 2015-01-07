// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "examples/window_manager/window_manager.mojom.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/common/common_type_converters.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/services/navigation/public/interfaces/navigation.mojom.h"
#include "mojo/services/view_manager/public/cpp/view.h"
#include "mojo/services/view_manager/public/cpp/view_manager.h"
#include "mojo/services/view_manager/public/cpp/view_manager_client_factory.h"
#include "mojo/services/view_manager/public/cpp/view_manager_delegate.h"
#include "mojo/views/native_widget_view_manager.h"
#include "mojo/views/views_init.h"
#include "ui/aura/client/focus_client.h"
#include "ui/aura/window.h"
#include "ui/events/event.h"
#include "ui/views/background.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/layout/layout_manager.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"
#include "url/gurl.h"

namespace mojo {
namespace examples {

class BrowserLayoutManager : public views::LayoutManager {
 public:
  BrowserLayoutManager() {}
  virtual ~BrowserLayoutManager() {}

 private:
  // Overridden from views::LayoutManager:
  virtual void Layout(views::View* host) override {
    // Browser view has one child, a text input field.
    DCHECK_EQ(1, host->child_count());
    views::View* text_field = host->child_at(0);
    gfx::Size ps = text_field->GetPreferredSize();
    text_field->SetBoundsRect(gfx::Rect(host->width(), ps.height()));
  }
  virtual gfx::Size GetPreferredSize(const views::View* host) const override {
    return gfx::Size();
  }

  DISALLOW_COPY_AND_ASSIGN(BrowserLayoutManager);
};

// This is the basics of creating a views widget with a textfield.
// TODO: cleanup!
class Browser : public ApplicationDelegate,
                public ViewManagerDelegate,
                public views::TextfieldController,
                public ViewObserver {
 public:
  Browser() : shell_(nullptr), root_(NULL), widget_(NULL) {}

  virtual ~Browser() {
    if (root_)
      root_->RemoveObserver(this);
  }

 private:
  // Overridden from ApplicationDelegate:
  virtual void Initialize(ApplicationImpl* app) override {
    shell_ = app->shell();
    view_manager_client_factory_.reset(
        new ViewManagerClientFactory(shell_, this));
    views_init_.reset(new ViewsInit);
    app->ConnectToService("mojo:window_manager", &window_manager_);
  }

  virtual bool ConfigureIncomingConnection(
      ApplicationConnection* connection) override {
    connection->AddService(view_manager_client_factory_.get());
    return true;
  }

  void CreateWidget(View* view) {
    views::Textfield* textfield = new views::Textfield;
    textfield->set_controller(this);

    views::WidgetDelegateView* widget_delegate = new views::WidgetDelegateView;
    widget_delegate->GetContentsView()->set_background(
        views::Background::CreateSolidBackground(SK_ColorBLUE));
    widget_delegate->GetContentsView()->AddChildView(textfield);
    widget_delegate->GetContentsView()->SetLayoutManager(
        new BrowserLayoutManager);

    widget_ = new views::Widget;
    views::Widget::InitParams params(
        views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
    params.native_widget = new NativeWidgetViewManager(widget_, shell_, view);
    params.delegate = widget_delegate;
    params.bounds = gfx::Rect(view->bounds().width, view->bounds().height);
    widget_->Init(params);
    widget_->Show();
    textfield->RequestFocus();
  }

  // ViewManagerDelegate:
  virtual void OnEmbed(View* root,
                       ServiceProviderImpl* exported_services,
                       scoped_ptr<ServiceProvider> imported_services) override {
    // TODO: deal with OnEmbed() being invoked multiple times.
    ConnectToService(imported_services.get(), &navigator_host_);
    root_ = root;
    root_->AddObserver(this);
    root_->SetFocus();
    CreateWidget(root_);
  }
  virtual void OnViewManagerDisconnected(
      ViewManager* view_manager) override {
    base::MessageLoop::current()->Quit();
  }

  // views::TextfieldController:
  virtual bool HandleKeyEvent(views::Textfield* sender,
                              const ui::KeyEvent& key_event) override {
    if (key_event.key_code() == ui::VKEY_RETURN) {
      GURL url(sender->text());
      if (!url.is_valid()) {
        LOG(ERROR) << "Not a valid URL: " << sender->text();
        return false;
      }
      printf("User entered this URL: %s\n", url.spec().c_str());
      URLRequestPtr request(URLRequest::New());
      request->url = String::From(url);
      navigator_host_->RequestNavigate(TARGET_NEW_NODE, request.Pass());
    }
    return false;
  }

  // ViewObserver:
  virtual void OnViewFocusChanged(View* gained_focus,
                                  View* lost_focus) override {
    aura::client::FocusClient* focus_client =
        aura::client::GetFocusClient(widget_->GetNativeView());
    if (lost_focus == root_)
      focus_client->FocusWindow(NULL);
    else if (gained_focus == root_)
      focus_client->FocusWindow(widget_->GetNativeView());
  }
  virtual void OnViewDestroyed(View* view) override {
    DCHECK_EQ(root_, view);
    view->RemoveObserver(this);
    root_ = NULL;
  }

  Shell* shell_;

  scoped_ptr<ViewsInit> views_init_;

  scoped_ptr<ViewManagerClientFactory> view_manager_client_factory_;
  View* root_;
  views::Widget* widget_;
  NavigatorHostPtr navigator_host_;
  IWindowManagerPtr window_manager_;

  DISALLOW_COPY_AND_ASSIGN(Browser);
};

}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new mojo::examples::Browser);
  return runner.Run(shell_handle);
}
