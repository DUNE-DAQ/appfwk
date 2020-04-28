#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_

/**
 * @file Buffer class interface which augments BufferI with push and pop
 * functions
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "BufferI.hh"

#include <chrono>
#include <cstddef>
#include <utility>
using std::size_t;

namespace appframework {

template <class T> class Buffer : public BufferI {

public:
  virtual size_t capacityBytes() const noexcept {
    return this->capacity() * sizeof(T);
  } /// bytes in buffer
};

template <class T, class DurationType = std::chrono::milliseconds>
class BufferInput : virtual public Buffer<T> {

public:
  template <typename U> void push(U &&t) {
    push_wait_for(std::forward<U>(t), DurationType(0));
  }

  virtual void push_wait_for(T &&, const DurationType &) = 0;

  void push_wait_for(const T &val, const DurationType &timeout) {
    push_wait_for(T(val), timeout);
  }
};

template <class T, class DurationType = std::chrono::milliseconds>
class BufferOutput : virtual public Buffer<T> {

public:
  T pop() { return pop_wait_for(DurationType(0)); }
  virtual T pop_wait_for(const DurationType &) = 0;
};

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_
