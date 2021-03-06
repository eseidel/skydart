#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
from utils import commit
from utils import mojo_root_dir
from utils import system

sdk_prefix_in_chromium = "third_party/mojo/src"
sdk_dirs_to_clone = [
  "mojo/edk",
  "mojo/public",
]


services_prefix_in_mojo = "mojo/services"
services_prefix_in_chromium = "third_party/mojo_services/src"

# Note: Services are placed under |services_prefix_in_chromium| at their
# location relative to |services_prefix_in_mojo|.
services_dirs_to_clone = [
  "mojo/services/public",
  "mojo/services/accessibility/public",
  "mojo/services/clipboard/public",
  "mojo/services/content_handler/public",
  "mojo/services/geometry/public",
  "mojo/services/gpu/public",
  "mojo/services/input_events/public",
  "mojo/services/native_viewport/public",
  "mojo/services/navigation/public",
  "mojo/services/surfaces/public",
  "mojo/services/view_manager/public",
  "mojo/services/window_manager/public",
]

# A dictionary mapping dirs to clone to their destination locations in Chromium.
dirs_to_clone = {}

for sdk_dir in sdk_dirs_to_clone:
  sdk_dir_in_chromium = os.path.join(sdk_prefix_in_chromium, sdk_dir)
  dirs_to_clone[sdk_dir] = sdk_dir_in_chromium

for service_dir in services_dirs_to_clone:
  service_relpath = os.path.relpath(service_dir, services_prefix_in_mojo)
  service_dir_in_chromium = os.path.join(services_prefix_in_chromium,
                                         service_relpath)
  dirs_to_clone[service_dir] = service_dir_in_chromium

def rev(source_dir, chromium_dir):
  src_commit = system(["git", "show-ref", "HEAD", "-s"], cwd=source_dir).strip()

  for input_dir, dest_dir in dirs_to_clone.iteritems():
    if os.path.exists(os.path.join(chromium_dir, dest_dir)):
      print "removing directory %s" % dest_dir
      system(["git", "rm", "-r", dest_dir], cwd=chromium_dir)
    print "cloning directory %s into %s" % (input_dir, dest_dir)
    files = system(["git", "ls-files", input_dir], cwd=source_dir)
    for f in files.splitlines():
      # Don't copy presubmit files over since the code is read-only on the
      # chromium side.
      if os.path.basename(f) == "PRESUBMIT.py":
        continue

      # Clone |f| into Chromium under |dest_dir| at its location relative to
      # |input_dir|.
      f_relpath = os.path.relpath(f, input_dir)
      dest_path = os.path.join(chromium_dir, dest_dir, f_relpath)
      system(["mkdir", "-p", os.path.dirname(dest_path)])
      system(["cp", os.path.join(source_dir, f), dest_path])
    os.chdir(chromium_dir)
    system(["git", "add", dest_dir], cwd=chromium_dir)

  mojo_public_dest_dir = os.path.join(sdk_prefix_in_chromium, "mojo/public")
  version_filename = os.path.join(mojo_public_dest_dir, "VERSION")
  with open(version_filename, "w") as version_file:
    version_file.write(src_commit)
  system(["git", "add", version_filename], cwd=chromium_dir)
  commit("Update mojo sdk to rev " + src_commit, cwd=chromium_dir)

if len(sys.argv) != 2:
  print "usage: rev_sdk.py <chromium source dir>"
  sys.exit(1)

rev(mojo_root_dir, sys.argv[1])
