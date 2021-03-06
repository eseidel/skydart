// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/common/data_pipe_utils.h"

#include <stdio.h>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/message_loop/message_loop.h"
#include "base/task_runner_util.h"
#include "base/threading/platform_thread.h"

namespace mojo {
namespace common {
namespace {

bool BlockingCopyHelper(ScopedDataPipeConsumerHandle source,
    const base::Callback<size_t(const void*, uint32_t)>& write_bytes) {
  for (;;) {
    const void* buffer = nullptr;
    uint32_t num_bytes = 0;
    MojoResult result = BeginReadDataRaw(
        source.get(), &buffer, &num_bytes, MOJO_READ_DATA_FLAG_NONE);
    if (result == MOJO_RESULT_OK) {
      size_t bytes_written = write_bytes.Run(buffer, num_bytes);
      if (bytes_written < num_bytes) {
        LOG(ERROR) << "write_bytes callback wrote fewer bytes ("
                   << bytes_written << ") written than expected (" << num_bytes
                   << ") in BlockingCopyHelper "
                   << "(pipe closed? out of disk space?)" << std::endl;
        return false;
      }
      result = EndReadDataRaw(source.get(), num_bytes);
      if (result != MOJO_RESULT_OK) {
        LOG(ERROR) << "EndReadDataRaw error (" << result
                   << ") in BlockingCopyHelper" << std::endl;
        return false;
      }
    } else if (result == MOJO_RESULT_SHOULD_WAIT) {
      result = Wait(source.get(),
                    MOJO_HANDLE_SIGNAL_READABLE,
                    MOJO_DEADLINE_INDEFINITE,
                    nullptr);
      if (result != MOJO_RESULT_OK) {
        // If the producer handle was closed, then treat as EOF.
        return result == MOJO_RESULT_FAILED_PRECONDITION;
      }
    } else if (result == MOJO_RESULT_FAILED_PRECONDITION) {
      // If the producer handle was closed, then treat as EOF.
      return true;
    } else {
      LOG(ERROR) << "Unhandled error " << result << " in BlockingCopyHelper"
                 << std::endl;
      // Some other error occurred.
      break;
    }
  }

  return false;
}

size_t CopyToStringHelper(
    std::string* result, const void* buffer, uint32_t num_bytes) {
  result->append(static_cast<const char*>(buffer), num_bytes);
  return num_bytes;
}

size_t CopyToFileHelper(FILE* fp, const void* buffer, uint32_t num_bytes) {
  return fwrite(buffer, 1, num_bytes, fp);
}

bool BlockingCopyFromFile(const base::FilePath& source,
                          ScopedDataPipeProducerHandle destination,
                          uint32_t skip) {
  base::File file(source, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!file.IsValid())
    return false;
  if (file.Seek(base::File::FROM_BEGIN, skip) != skip) {
    return false;
  }
  for (;;) {
    void* buffer = nullptr;
    uint32_t buffer_num_bytes = 0;
    MojoResult result =
        BeginWriteDataRaw(destination.get(), &buffer, &buffer_num_bytes,
                          MOJO_WRITE_DATA_FLAG_NONE);
    if (result == MOJO_RESULT_OK) {
      int bytes_read =
          file.ReadAtCurrentPos(static_cast<char*>(buffer), buffer_num_bytes);
      if (bytes_read >= 0) {
        EndWriteDataRaw(destination.get(), bytes_read);
        if (bytes_read == 0) {
          // eof
          return true;
        }
      } else {
        // error
        EndWriteDataRaw(destination.get(), 0);
        return false;
      }
    } else if (result == MOJO_RESULT_SHOULD_WAIT) {
      result = Wait(destination.get(), MOJO_HANDLE_SIGNAL_WRITABLE,
                    MOJO_DEADLINE_INDEFINITE, nullptr);
      if (result != MOJO_RESULT_OK) {
        // If the consumer handle was closed, then treat as EOF.
        return result == MOJO_RESULT_FAILED_PRECONDITION;
      }
    } else {
      // If the consumer handle was closed, then treat as EOF.
      return result == MOJO_RESULT_FAILED_PRECONDITION;
    }
  }
#if !defined(OS_WIN)
  NOTREACHED();
  return false;
#endif
}

} // namespace

// TODO(hansmuller): Add a max_size parameter.
bool BlockingCopyToString(ScopedDataPipeConsumerHandle source,
                          std::string* result) {
  CHECK(result);
  result->clear();
  return BlockingCopyHelper(
      source.Pass(), base::Bind(&CopyToStringHelper, result));
}

bool BlockingCopyToFile(ScopedDataPipeConsumerHandle source,
                        const base::FilePath& destination) {
  base::ScopedFILE fp(base::OpenFile(destination, "wb"));
  if (!fp) {
    LOG(ERROR) << "OpenFile('" << destination.value()
               << "'failed in BlockingCopyToFile" << std::endl;
    return false;
  }
  return BlockingCopyHelper(
      source.Pass(), base::Bind(&CopyToFileHelper, fp.get()));
}

void CopyToFile(ScopedDataPipeConsumerHandle source,
                const base::FilePath& destination,
                base::TaskRunner* task_runner,
                const base::Callback<void(bool)>& callback) {
  base::PostTaskAndReplyWithResult(
      task_runner,
      FROM_HERE,
      base::Bind(&BlockingCopyToFile, base::Passed(&source), destination),
      callback);
}

void CopyFromFile(const base::FilePath& source,
                  ScopedDataPipeProducerHandle destination,
                  uint32_t skip,
                  base::TaskRunner* task_runner,
                  const base::Callback<void(bool)>& callback) {
  base::PostTaskAndReplyWithResult(task_runner, FROM_HERE,
                                   base::Bind(&BlockingCopyFromFile, source,
                                              base::Passed(&destination), skip),
                                   callback);
}

}  // namespace common
}  // namespace mojo
