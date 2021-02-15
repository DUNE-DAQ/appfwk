/**
 * @file DAQSource.hpp DAQSource class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQSOURCE_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQSOURCE_HPP_

#include "appfwk/Queue.hpp"
#include "appfwk/QueueRegistry.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/DAQModuleHelper.hpp"
#include "appfwk/cmd/Structs.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "TRACE/trace.h"
#include "ers/Issue.h"

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
class DAQSource : public Named
{
public:
  using value_t = T;
  using duration_t = std::chrono::milliseconds;

  explicit DAQSource(const std::string& inst);
  DAQSource(const nlohmann::json& init_json, const std::string& name);
  DAQSource(const cmd::ModInit& init, const std::string& name);

  void pop(T&, const duration_t& timeout = duration_t::zero());
  bool can_pop() const noexcept;
  const std::string& get_name() const final { return m_queue->get_name(); }

  DAQSource(DAQSource const&) = delete;
  DAQSource(DAQSource&&) = delete;
  DAQSource& operator=(DAQSource const&) = delete;
  DAQSource& operator=(DAQSource&&) = delete;

private:
  std::shared_ptr<Queue<T>> m_queue;
};

template<typename T>
DAQSource<T>::DAQSource(const std::string& inst)
{
  try {
    m_queue = QueueRegistry::get().get_queue<T>(inst);
    TLOG(TLVL_TRACE, "DAQSource") << "Queue " << inst << " is at " << m_queue.get();
  } catch (QueueTypeMismatch& ex) {
    throw DAQSourceConstructionFailed(ERS_HERE, inst, ex);
  }
}

template<typename T>
DAQSource<T>::DAQSource(const nlohmann::json& init_json, const std::string& name)
{
  try {
    auto qindex=queue_index(init_json, {name});
    m_queue = QueueRegistry::get().get_queue<T>(qindex[name].inst);
    TLOG(TLVL_TRACE, "DAQSource") << "Queue " << name << " is at " << m_queue.get();
  } catch (const QueueTypeMismatch& ex) {
    throw DAQSourceConstructionFailed(ERS_HERE, name, ex);
  }
  catch (const SchemaError& ex) {
    throw DAQSourceConstructionFailed(ERS_HERE, name, ex);
  }
}

template<typename T>
DAQSource<T>::DAQSource(const cmd::ModInit& init, const std::string& name)
{
  nlohmann::json init_json=init;
  try {
    auto qindex=queue_index(init_json, {name});
    m_queue = QueueRegistry::get().get_queue<T>(qindex[name].inst);
    TLOG(TLVL_TRACE, "DAQSource") << "Queue " << name << " is at " << m_queue.get();
  } catch (const QueueTypeMismatch& ex) {
    throw DAQSourceConstructionFailed(ERS_HERE, name, ex);
  }
  catch (const SchemaError& ex) {
    throw DAQSourceConstructionFailed(ERS_HERE, name, ex);
  }
}

template<typename T>
void
DAQSource<T>::pop(T& val, const duration_t& timeout)
{
  m_queue->pop(val, timeout);
}

template<typename T>
bool
DAQSource<T>::can_pop() const noexcept
{
  return m_queue->can_pop();
}

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_DAQSOURCE_HPP_
