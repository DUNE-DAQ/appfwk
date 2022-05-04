/**
 * @file DAQSink.hpp DAQSink class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQSINK_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQSINK_HPP_

#include "iomanager/IOManager.hpp"

#include "logging/Logging.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <utility>

namespace dunedaq {
// Disable coverage collection LCOV_EXCL_START
/**
 * @brief Define an ERS Issue for when DAQSink is unable to retrieve its Queue
 * handle
 */
ERS_DECLARE_ISSUE(appfwk,                                           // namespace
                  DAQSinkConstructionFailed,                        // issue class name
                  "Failed to construct DAQSink \"" << name << "\"", // no message
                  ((std::string)name))
// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {

using iomanager::QueueTimeoutExpired;

template<typename T>
class [[deprecated("Use iomanager::Sender instead")]] DAQSink : public utilities::Named
{
public:
  using value_t = T;
  using duration_t = std::chrono::milliseconds;

  explicit DAQSink(const std::string& name);
  void push(T&& element, const duration_t& timeout = duration_t::zero());
  void push(const T& element, const duration_t& timeout = duration_t::zero());
  bool can_push() const noexcept;
  const std::string& get_name() const final { return m_queue->get_name(); }

  DAQSink(DAQSink const&) = delete;
  DAQSink(DAQSink&&) = delete;
  DAQSink& operator=(DAQSink const&) = delete;
  DAQSink& operator=(DAQSink&&) = delete;

private:
  std::shared_ptr<iomanager::SenderConcept<T>> m_queue;
};

template<typename T>
DAQSink<T>::DAQSink(const std::string& name)
{
  try {
    m_queue = get_iom_sender<T>(name);
    TLOG_DEBUG(1, "DAQSink") << "Queue " << name << " is at " << m_queue.get();
  } catch (const ers::Issue& ex) {
    throw DAQSinkConstructionFailed(ERS_HERE, name, ex);
  }
}

template<typename T>
void
DAQSink<T>::push(T&& element, const duration_t& timeout)
{
  m_queue->send(element, timeout);
}

template<typename T>
void
DAQSink<T>::push(const T& element, const duration_t& timeout)
{
  auto copy = T(element);
  m_queue->send(copy, timeout);
}

template<typename T>
bool
DAQSink<T>::can_push() const noexcept
{
#pragma message("This method gives false results, use try..catch to check if data was actually sent")
  return true;
}

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_DAQSINK_HPP_
