/**
 * @file FanOutDAQModule.hh
 *
 * FanOutDAQModule is a simple DAQModule implementation that simply logs the
 * fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_FANOUTDAQMODULE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_FANOUTDAQMODULE_HH_

#include "app-framework/DAQModule.hh"
#include "app-framework/DAQModuleThreadHelper.hh"
#include "app-framework/DAQSink.hh"
#include "app-framework/DAQSource.hh"

#include "TRACE/trace.h"

#include <limits>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace appframework {

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
};

/**
 * @brief FanOutDAQModule sends data to multiple Queues
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
    Broadcast, ///< FanOutDAQModule will copy elements from input to all outputs
    RoundRobin, ///< FanOutDAQModule will distribute elements from input to outputs in a round-robin fashion
    FirstAvailable ///< FanOutDAQModule will distribute elements from input to the first available output
  };

  /**
   * @brief Execute a command from DAQProcess
   * @param cmd Command from DAQProcess
   * @param args Arguments for the command from DAQProcess
   */
  void execute_command(const std::string& cmd,
                       const std::vector<std::string>& args = {}) override;

  FanOutDAQModule(const FanOutDAQModule&) = delete; ///< FanOutDAQModule is not copy-constructible
  FanOutDAQModule& operator=(const FanOutDAQModule&) = delete; ///< FanOutDAQModule is not copy-assignable
  FanOutDAQModule(FanOutDAQModule&&) = delete; ///< FanOutDAQModule is not move-constructible
  FanOutDAQModule& operator=(FanOutDAQModule&&) = delete; ///< FanOutDAQModule is not move-assignable

private:
  // Commands
  std::string do_configure();
  std::string do_start();
  std::string do_stop();

  // Type traits handling. Yes, the "U" template parameter is actually
  // necessary, even though it's just an alias to this user module's
  // data type.

  template<typename U = ValueType>
  typename std::enable_if_t<!std::is_copy_constructible_v<U>> do_broadcast(
    ValueType&) const
  {
    throw std::runtime_error(
      "Broadcast mode cannot be used for non-copy-constructible types!");
  }
  template<typename U = ValueType>
  typename std::enable_if_t<std::is_copy_constructible_v<U>> do_broadcast(
    ValueType& data) const
  {
    for (auto& o : outputQueues_) {
      auto starttime = std::chrono::steady_clock::now();
      o->push(ValueType(data), queueTimeout_);
      auto endtime = std::chrono::steady_clock::now();
      if (std::chrono::duration_cast<decltype(queueTimeout_)>(
            endtime - starttime) > queueTimeout_) {
        TLOG(TLVL_WARNING) << "Timeout occurred trying to broadcast data to "
                              "output queue; data may be lost if it doesn't "
                              "make it into any other output queues, either";
      }
    }
  }

  // Threading
  void do_work();
  DAQModuleThreadHelper thread_;

  // Configuration
  FanOutMode mode_;
  std::chrono::milliseconds queueTimeout_;

  std::unique_ptr<DAQSource<ValueType>> inputQueue_;
  std::list<std::unique_ptr<DAQSink<ValueType>>> outputQueues_;
  size_t wait_interval_us_;
};
} // namespace appframework

#include "detail/FanOutDAQModule.icc"

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_FANOUTDAQMODULE_HH_
