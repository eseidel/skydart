// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_SURFACES_APP_CHILD_GL_IMPL_H_
#define EXAMPLES_SURFACES_APP_CHILD_GL_IMPL_H_

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/time/time.h"
#include "cc/surfaces/surface_id.h"
#include "examples/sample_app/spinning_cube.h"
#include "examples/surfaces_app/child.mojom.h"
#include "mojo/public/c/gles2/gles2.h"
#include "mojo/public/cpp/bindings/string.h"
#include "mojo/services/surfaces/public/interfaces/surface_id.mojom.h"
#include "mojo/services/surfaces/public/interfaces/surfaces.mojom.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/size.h"

namespace cc {
class CompositorFrame;
}

namespace mojo {

class ApplicationConnection;

namespace examples {

// Simple example of a child app using surfaces + GL.
class ChildGLImpl : public InterfaceImpl<Child>, public ResourceReturner {
 public:
  ChildGLImpl(ApplicationConnection* surfaces_service_connection,
              CommandBufferPtr command_buffer);
  ~ChildGLImpl() override;

 private:
  using ProduceCallback = mojo::Callback<void(SurfaceIdPtr id)>;

  // Child implementation.
  void ProduceFrame(ColorPtr color,
                    SizePtr size,
                    const ProduceCallback& callback) override;

  // ResourceReturner implementation
  void ReturnResources(Array<ReturnedResourcePtr> resources) override;

  void SetIdNamespace(uint32_t id_namespace);
  void Draw();
  void RunProduceCallback();

  SkColor color_;
  gfx::Size size_;
  SurfacePtr surface_;
  MojoGLES2Context context_;
  uint32_t id_namespace_;
  uint32_t local_id_;
  ::examples::SpinningCube cube_;
  base::TimeTicks start_time_;
  uint32_t next_resource_id_;
  base::hash_map<uint32_t, GLuint> id_to_tex_map_;
  ProduceCallback produce_callback_;
  Binding<ResourceReturner> returner_binding_;

  DISALLOW_COPY_AND_ASSIGN(ChildGLImpl);
};

}  // namespace examples
}  // namespace mojo

#endif  // EXAMPLES_SURFACES_APP_CHILD_GL_IMPL_H_
