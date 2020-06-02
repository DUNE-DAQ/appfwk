/**
 * @file DAQSource.hh DAQSource class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSOURCE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSOURCE_HH_

#include "TRACE/trace.h"
#include "ers/Issue.h"
#include <app-framework/QueueI.hh>
#include <app-framework/QueueRegistry.hh>
#include <chrono>
#include <memory>
#include <string>
#include <typeinfo>

ERS_DECLARE_ISSUE(appframework,               // namespace
                  DAQSourceConstrutionFailed, // issue class name
                  "Failed to construct DAQSource \"" << name
                                                     << "\"", // no message
                  ((std::string)name))

namespace appframework {

template<typename T>
class DAQSource
{
public:
  using value_type = T;
  using duration_type = std::chrono::milliseconds;

  explicit DAQSource(std::string name);
  T pop(const duration_type& timeout = duration_type::zero());
  bool can_pop();

private:
  std::shared_ptr<QueueI<T>> queue_;
};

template<typename T>
DAQSource<T>::DAQSource(std::string name)
{
  try {
    queue_ = QueueRegistry::get()->get_queue<T>(name);
    TLOG(TLVL_TRACE, "DAQSource")
      << "Queue " << name << " is at " << queue_.get();
  } catch (QueueTypeMismatch& ex) {
    throw DAQSourceConstrutionFailed(ERS_HERE, name, ex);
  }
}

template<typename T>
T
DAQSource<T>::pop(const duration_type& timeout)
{
  return queue_->pop(timeout);
}

template<typename T>
bool
DAQSource<T>::can_pop()
{
  return queue_->can_pop();
}

} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQSOURCE_HH_