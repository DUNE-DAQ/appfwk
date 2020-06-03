/**
 * @file Queue class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUEI_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUEI_HH_

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "app-framework/NamedObject.hh"

namespace appframework {

/**
 * @brief Implementations of the Queue class are responsible for relaying data
 * between DAQModules within a DAQ Application
 *
 * Note that while the Queue class itself is not templated on a data type (so
 * it can be included in generic containers), all implementations should be.
 */
template<class T>
class QueueI : public NamedObject
{
public:
  using value_type = T;
  using duration_type = std::chrono::milliseconds;

  QueueI(std::string name)
    : NamedObject(name)
  {}

  virtual void push(T&& val, const duration_type& timeout) = 0;

  virtual bool can_push() const noexcept = 0;

  virtual T pop(const duration_type& timeout) = 0;

  virtual bool can_pop() const noexcept = 0;

private:
  QueueI(const QueueI&) = delete;
  QueueI& operator=(const QueueI&) = delete;
  QueueI(QueueI&&) = default;
  QueueI& operator=(QueueI&&) = default;
};

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUEI_HH_