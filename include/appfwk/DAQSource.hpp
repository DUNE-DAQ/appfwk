/**
 * @file DAQSource.hpp DAQSource class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQSOURCE_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQSOURCE_HPP_

#include "iomanager/IOManager.hpp"

#include "logging/Logging.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <typeinfo>

namespace dunedaq {
// Disable coverage collection LCOV_EXCL_START
/**
 * @brief Define an ERS Issue for when DAQSource is unable to retrieve its Queue
 * handle
 */
ERS_DECLARE_ISSUE(appfwk,                                             // namespace
                  DAQSourceConstructionFailed,                        // issue class name
                  "Failed to construct DAQSource \"" << name << "\"", // no message
                  ((std::string)name))
// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {

using iomanager::QueueTimeoutExpired;

template<typename T>
class [[deprecated("Use iomanager::Receiver instead")]] DAQSource : public utilities::Named
{
public:
  using value_t = T;
  using duration_t = std::chrono::milliseconds;

  explicit DAQSource(const std::string& name);
  void pop(T&, const duration_t& timeout = duration_t::zero());
  bool can_pop() const noexcept;
  const std::string& get_name() const final { return m_queue->get_name(); }

  DAQSource(DAQSource const&) = delete;
  DAQSource(DAQSource &&) = delete;
  DAQSource& operator=(DAQSource const&) = delete;
  DAQSource& operator=(DAQSource&&) = delete;

private:
  std::shared_ptr<iomanager::ReceiverConcept<T>> m_queue;
};

template<typename T>
DAQSource<T>::DAQSource(const std::string& name)
{
  try {
    iomanager::IOManager iom;
    m_queue = iom.get_receiver<T>(name);
    TLOG_DEBUG(1, "DAQSource") << "Queue " << name << " is at " << m_queue.get();
  } catch (ers::Issue& ex) {
    throw DAQSourceConstructionFailed(ERS_HERE, name, ex);
  }
}

template<typename T>
void
DAQSource<T>::pop(T& val, const duration_t& timeout)
{
  val = m_queue->receive(timeout);
}

template<typename T>
bool
DAQSource<T>::can_pop() const noexcept
{
#pragma message("This method gives false results, use try..catch to see if data was received")
  return true;
}

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_DAQSOURCE_HPP_
