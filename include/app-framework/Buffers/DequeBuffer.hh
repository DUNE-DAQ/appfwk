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
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

namespace appframework {

template <class ValueType, class DurationType = std::chrono::milliseconds>
class DequeBuffer : public BufferInput<ValueType, DurationType>,
                    public BufferOutput<ValueType, DurationType> {
public:
  using value_type = ValueType;
  using duration_type = DurationType;

  // This is needed in order for all signatures of the functions of
  // this name in the base class to be accessible here, given that a
  // subset are overridden

  using BufferInput<ValueType, DurationType>::push;

  DequeBuffer();

  void Configure();

  size_t size() const noexcept { return fSize.load(); }
  size_t capacity() const noexcept override { return fCapacity; }

  bool empty() const noexcept override { return size() == 0; }
  bool full() const noexcept override { return size() >= capacity(); }

  value_type pop(const duration_type &timeout)
      override; // Throws std::runtime_error if a timeout occurs
  void push(value_type &&val, const duration_type &timeout)
      override; // Throws std::runtime_error if a timeout occurs

  // Delete the copy and move operations since various member data instances
  // (e.g., of std::mutex or of std::atomic) aren't copyable or movable

  DequeBuffer(const DequeBuffer&) = delete;
  DequeBuffer& operator=(const DequeBuffer&) = delete;
  DequeBuffer(DequeBuffer&&) = delete;
  DequeBuffer& operator=(DequeBuffer&&) = delete;

private:
  void try_lock_for(std::unique_lock<std::mutex> &, const duration_type &);

  std::deque<value_type> fDeque;
  std::atomic<size_t> fSize = 0;
  size_t fCapacity;

  std::mutex fMutex;
  std::condition_variable fNoLongerFull;
  std::condition_variable fNoLongerEmpty;
};

#include "detail/DequeBuffer.icc"

} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_
