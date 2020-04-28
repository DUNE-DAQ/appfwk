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

template <class T> class BufferInput : virtual public Buffer<T> {

   public:
  explicit BufferInput(size_t pt = 10) : fPushTimeout_ms(pt) {}

    virtual int push(const T& t) { return push(std::move(t)); }
    virtual int push(T&&) = 0;

  size_t get_push_timeout() const { return fPushTimeout_ms; }
  void set_push_timeout(size_t sz) { fPushTimeout_ms = sz; }

  BufferInput(const BufferInput &) = default;
  BufferInput &operator=(const BufferInput &) = default;
  BufferInput(BufferInput &&) = default;
  BufferInput &operator=(BufferInput &&) = default;

private:
    size_t fPushTimeout_ms;
};

template <class T> class BufferOutput : virtual public Buffer<T> {

   public:
  explicit BufferOutput(size_t pt = 10) : fPopTimeout_ms(pt) {}

    virtual T pop() = 0;

  size_t get_pop_timeout() const { return fPopTimeout_ms; }
  void set_pop_timeout(size_t sz) { fPopTimeout_ms = sz; }

  BufferOutput(const BufferOutput &) = default;
  BufferOutput &operator=(const BufferOutput &) = default;
  BufferOutput(BufferOutput &&) = default;
  BufferOutput &operator=(BufferOutput &&) = default;

private:
    size_t fPopTimeout_ms;
};

}  // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_
