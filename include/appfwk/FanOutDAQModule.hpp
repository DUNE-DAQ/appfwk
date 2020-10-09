/**
 * @file FanOutDAQModule.hpp
 *
 * FanOutDAQModule is a simple DAQModule implementation that simply logs the
 * fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_FANOUTDAQMODULE_HPP_
#define APPFWK_INCLUDE_APPFWK_FANOUTDAQMODULE_HPP_

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSink.hpp"
#include "appfwk/DAQSource.hpp"
#include "appfwk/ThreadHelper.hpp"

#include "TRACE/trace.h"
#include <ers/Issue.h>

#include <limits>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace dunedaq {

/**
 * @brief The BroadcastFailed FanOutDAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,                                        ///< Namespace
                       BroadcastFailed,                               ///< Type of the Issue
                       GeneralDAQModuleIssue,                         ///< Base class of the Issue
                       "FanOutDAQModule Broadcast Error: " << reason, ///< Log Message from the issue
                       ((std::string)name),                           ///< End of variable declarations
                       ((std::string)reason))                         ///< Variables to capture

/**
 * @brief The ConfigureFailed FanOutDAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,                                        ///< Namespace
                       ConfigureFailed,                               ///< Type of the Issue
                       GeneralDAQModuleIssue,                         ///< Base class of the Issue
                       "FanOutDAQModule Configure Error: " << reason, ///< Log Message from the issue
                       ((std::string)name),                           ///< End of variable declarations
                       ((std::string)reason))                         ///< Variables to capture

namespace appfwk {

/**
 * @brief FanOutDAQModule sends data to multiple Queues
 * The memory that can be transported via a FanOut Moduele
 * requires an empty constructor
 */
template<typename ValueType>
class FanOutDAQModule : public DAQModule
{
public:
  /**
   * @brief FanOutDAQModule Constructor
   * @param name Name of this FanOutDAQModule instance
   */
  explicit FanOutDAQModule(std::string name);

  /**
   * @brief Defines the possible modes for FanOutDAQModule
   */
  enum class FanOutMode
  {
    NotConfigured, ///< FanOutDAQModule is not configured
    Broadcast,     ///< FanOutDAQModule will copy elements from input to all outputs
    RoundRobin,    ///< FanOutDAQModule will distribute elements from input to
                   ///< outputs in a round-robin fashion
    FirstAvailable ///< FanOutDAQModule will distribute elements from input to
                   ///< the first available output
  };

  /**
   * @brief Execute a command from DAQProcess
   * @param cmd Command from DAQProcess
   * @param args Arguments for the command from DAQProcess
   */
  // void execute_command(const std::string& cmd, const data_t& data = {}) override;

  FanOutDAQModule(const FanOutDAQModule&) = delete;            ///< FanOutDAQModule is not copy-constructible
  FanOutDAQModule& operator=(const FanOutDAQModule&) = delete; ///< FanOutDAQModule is not copy-assignable
  FanOutDAQModule(FanOutDAQModule&&) = delete;                 ///< FanOutDAQModule is not move-constructible
  FanOutDAQModule& operator=(FanOutDAQModule&&) = delete;      ///< FanOutDAQModule is not move-assignable

  void init(const nlohmann::json& ) override;

private:
  // Commands
  void do_configure(const data_t& data);
  void do_start(const data_t& data);
  void do_stop(const data_t& data);

  // Type traits handling. Yes, the "U" template parameter is actually
  // necessary, even though it's just an alias to this user module's
  // data type.

  template<typename U = ValueType>
  typename std::enable_if_t<!std::is_copy_constructible_v<U>> do_broadcast(ValueType&) const
  {
    throw BroadcastFailed(ERS_HERE, get_name(), "Broadcast mode cannot be used for non-copy-constructible types!");
  }
  template<typename U = ValueType>
  typename std::enable_if_t<std::is_copy_constructible_v<U>> do_broadcast(ValueType& data) const
  {
    for (auto& o : outputQueues_) {
      try {
	o->push(data, queueTimeout_);
      } catch (const QueueTimeoutExpired& ex) {
        ers::warning(BroadcastFailed(ERS_HERE,
                                     get_name(),
                                     "Timeout occurred trying to broadcast data to "
                                     "output queue; data may be lost if it doesn't "
                                     "make it into any other output queues, either"));
      }
    }
  }

  // Threading
  void do_work(std::atomic<bool>&);
  ThreadHelper thread_;

  // Configuration
  FanOutMode mode_;
  std::chrono::milliseconds queueTimeout_;

  std::unique_ptr<DAQSource<ValueType>> inputQueue_;
  std::list<std::unique_ptr<DAQSink<ValueType>>> outputQueues_;
  size_t wait_interval_us_;
};

/**
 * @brief Struct used for FanOutDAQModule_test
 */
struct NonCopyableType
{
  int data; ///< Integer data for NonCopyableType
  /**
   * @brief NonCopyableType Constructor
   * @param d Initialization for data member
   */
  explicit NonCopyableType(int d)
    : data(d)
  {}
  NonCopyableType(NonCopyableType const&) = delete; ///< NonCopyableType is not copy-constructible
  /**
   * @brief Move Constructor for NonCopyableType
   * @param i NonCopyableType rvalue to move from
   */
  NonCopyableType(NonCopyableType&& i) { data = i.data; }
  NonCopyableType& operator=(NonCopyableType const&) = delete; ///< NonCopyableType is not copy-assignable
  /**
   * @brief Move assignment operator for NonCopyableType
   * @param i NonCopyableType to move from
   * @return NonCopyableType instance
   */
  NonCopyableType& operator=(NonCopyableType&& i)
  {
    data = i.data;
    return *this;
  }

private:
  NonCopyableType()
    : data(0)
  {}

  friend class FanOutDAQModule<NonCopyableType>;
};

} // namespace appfwk
} // namespace dunedaq

#include "detail/FanOutDAQModule.hxx"

#endif // APPFWK_INCLUDE_APPFWK_FANOUTDAQMODULE_HPP_
