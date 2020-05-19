#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUE_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUE_HH_

/**
 * @file Queue class interface which augments QueueI introducint a templated
 * type for the base memory type of the transport method between DAQModules
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "QueueI.hh"

#include <chrono>
#include <cstddef>
#include <utility>
using std::size_t;

namespace appframework {

template<class ValueType, class DurationType = std::chrono::milliseconds>
class QueueSink : virtual public QueueI // NOLINT
{

public:
  virtual void push(ValueType&& val, const DurationType& timeout) = 0;

  // To use the non-virtual push which leaves its value
  // argument unchanged, make sure to add

  // using QueueSink<ValueType,DurationType>::push

  // in your derived class declaration (line above assumes you're
  // using the same template parameter labels)

  void push(const ValueType& val, const DurationType& timeout)
  {
    push(ValueType(val), timeout);
  }

  virtual bool can_push() const noexcept = 0;
};

template<class ValueType, class DurationType = std::chrono::milliseconds>
class QueueSource : virtual public QueueI // NOLINT
{

public:
  virtual ValueType pop(const DurationType& timeout) = 0;

  virtual bool can_pop() const noexcept = 0;
};

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUE_HH_
