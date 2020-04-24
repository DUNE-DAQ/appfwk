#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_

/**
 *
 * @file A deque
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework-base/Buffers/Buffer.hh"

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <utility>
#include <unistd.h>


namespace appframework {

template <class T>
class DequeBuffer : public BufferInput<T>, public BufferOutput<T> {
public:
  DequeBuffer();

  void Configure();

  size_t size() const noexcept { return fSize.load(); }
  size_t capacity() const noexcept override { return fCapacity; }

  bool empty() const noexcept override { return size() == 0; }
  bool full() const noexcept override { return size() >= capacity(); }

  int push(T &&); /// push one on, return new size if successful, -1 if not
  T pop();        /// pop one off, return object

  // Delete the copy and move operations since member data instances
  // of std::mutex and std::atomic aren't copyable or movable

  DequeBuffer(const DequeBuffer &) = delete;
  DequeBuffer &operator=(const DequeBuffer &) = delete;
  DequeBuffer(DequeBuffer &&) = delete;
  DequeBuffer &operator=(DequeBuffer &&) = delete;

private:
  std::deque<T> fDeque;
  std::atomic<size_t> fSize = 0;
  size_t fCapacity;

  std::mutex fMutex;
  size_t fRetryTime_ms;
  size_t fPushRetries;
  size_t fPopRetries;
};

#include "detail/DequeBuffer.icc"

} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_
