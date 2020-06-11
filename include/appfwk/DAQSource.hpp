/**
 * @file DAQSource.hpp DAQSource class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSOURCE_HPP_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSOURCE_HPP_

#include "TRACE/trace.h"
#include "ers/Issue.h"
#include <appfwk/Queue.hpp>
#include <appfwk/QueueRegistry.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <typeinfo>

namespace dunedaq {
/**
 * @brief Define an ERS Issue for when DAQSource is unable to retrieve its Queue
 * handle
 */
ERS_DECLARE_ISSUE(appfwk,                                             // namespace
                  DAQSourceConstructionFailed,                        // issue class name
                  "Failed to construct DAQSource \"" << name << "\"", // no message
                  ((std::string)name))

namespace appfwk {

template<typename T>
class DAQSource
{
public:
  using value_type = T;
  using duration_type = std::chrono::milliseconds;

  explicit DAQSource(const std::string& name);
  bool pop(T&, const duration_type& timeout = duration_type::zero());
  bool can_pop();

private:
  std::shared_ptr<Queue<T>> queue_;
};

template<typename T>
DAQSource<T>::DAQSource(const std::string& name)
{
  try {
    queue_ = QueueRegistry::get().get_queue<T>(name);
    TLOG(TLVL_TRACE, "DAQSource") << "Queue " << name << " is at " << queue_.get();
  } catch (QueueTypeMismatch& ex) {
    throw DAQSourceConstructionFailed(ERS_HERE, name, ex);
  }
}

template<typename T>
bool
DAQSource<T>::pop(T& val, const duration_type& timeout)
{
  return queue_->pop(val, timeout);
}

template<typename T>
bool
DAQSource<T>::can_pop()
{
  return queue_->can_pop();
}

} // namespace appfwk
} // namespace dunedaq

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSOURCE_HPP_
