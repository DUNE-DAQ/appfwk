/**
 * @file DAQSink.hpp DAQSink class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSINK_HPP_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSINK_HPP_

#include "appfwk/Queue.hpp"
#include "appfwk/QueueRegistry.hpp"

#include "TRACE/trace.h"
#include "ers/Issue.h"

#include <chrono>
#include <memory>
#include <string>
#include <utility>

namespace dunedaq {
/**
 * @brief Define an ERS Issue for when DAQSink is unable to retrieve its Queue
 * handle
 */
ERS_DECLARE_ISSUE(appfwk,                                           // namespace
                  DAQSinkConstructionFailed,                        // issue class name
                  "Failed to construct DAQSink \"" << name << "\"", // no message
                  ((std::string)name))

namespace appfwk {

template<typename T>
class DAQSink
{
public:
  using value_type = T;
  using duration_type = std::chrono::milliseconds;

  explicit DAQSink(const std::string& name);
  void push(T&& element, const duration_type& timeout = duration_type::zero());
  void push(const T& element, const duration_type& timeout = duration_type::zero());
  bool can_push();

private:
  std::shared_ptr<Queue<T>> queue_;
};

template<typename T>
DAQSink<T>::DAQSink(const std::string& name)
{
  try {
    queue_ = QueueRegistry::get().get_queue<T>(name);
    TLOG(TLVL_TRACE, "DAQSink") << "Queue " << name << " is at " << queue_.get();
  } catch (QueueTypeMismatch& ex) {
    throw DAQSinkConstructionFailed(ERS_HERE, name, ex);
  }
}

template<typename T>
void
DAQSink<T>::push(T&& element, const duration_type& timeout)
{
  queue_->push(std::move(element), timeout);
}

template<typename T>
void
DAQSink<T>::push(const T& element, const duration_type& timeout)
{
  queue_->push( T(element), timeout);
}

template<typename T>
bool
DAQSink<T>::can_push()
{
  return queue_->can_push();
}

} // namespace appfwk
} // namespace dunedaq

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSINK_HPP_
