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

template<class T>
class Buffer : public BufferI
{

public:
  virtual size_t capacityBytes() const noexcept
  {
    return this->capacity() * sizeof(T);
  } /// bytes in buffer
};

template <class ValueType, class DurationType = std::chrono::milliseconds>
class BufferInput : virtual public Buffer<ValueType> {

public:
  virtual void push(ValueType &&val, const DurationType &timeout) = 0;

  // To use the non-virtual push which leaves its value
  // argument unchanged, make sure to add
  // using BufferInput<ValueType,DurationType>::push
  // in your derived class declaration (line above assumes you're
  // using the same template parameter labels)

  void push(const ValueType &val, const DurationType &timeout) {
    push(ValueType(val), timeout);
  }
};

template <class ValueType, class DurationType = std::chrono::milliseconds>
class BufferOutput : virtual public Buffer<ValueType> {

public:
  virtual ValueType pop(const DurationType &timeout) = 0;
};

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_
