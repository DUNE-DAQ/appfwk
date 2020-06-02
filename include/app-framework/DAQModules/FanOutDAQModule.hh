/**
 * @file The FanOutDAQModule class interface
 *
 * FanOutDAQModule is a simple DAQModule implementation that simply logs the
 * fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FANOUTDAQMODULE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FANOUTDAQMODULE_HH_

#include "app-framework/DAQModules/DAQModuleI.hh"
#include "app-framework/DAQModules/DAQModuleThreadHelper.hh"
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
  int data;
  explicit NonCopyableType(int d)
    : data(d)
  {}
  NonCopyableType(NonCopyableType const&) = delete;
  NonCopyableType(NonCopyableType&& i) { data = i.data; }
  NonCopyableType& operator=(NonCopyableType const&) = delete;
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
class FanOutDAQModule : public DAQModuleI
{
public:
  FanOutDAQModule(std::string name);

  enum class FanOutMode
  {
    NotConfigured,
    Broadcast,
    RoundRobin,
    FirstAvailable
  };

  /**
   * @brief Logs the reception of the command
   * @param cmd Command from DAQProcess
   */
  void execute_command(const std::string& cmd,
                       const std::vector<std::string>& args = {}) override;

  FanOutDAQModule(const FanOutDAQModule&) = delete;
  FanOutDAQModule& operator=(const FanOutDAQModule&) = delete;
  FanOutDAQModule(FanOutDAQModule&&) = delete;
  FanOutDAQModule& operator=(FanOutDAQModule&&) = delete;

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

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FANOUTDAQMODULE_HH_
