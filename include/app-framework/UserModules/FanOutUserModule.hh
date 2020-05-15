/**
 * @file The FanOutUserModule class interface
 *
 * FanOutUserModule is a simple UserModule implementation that simply logs the
 * fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_USERMODULES_FANOUTUSERMODULE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_USERMODULES_FANOUTUSERMODULE_HH_

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/UserModules/UserModule.hh"
#include "app-framework-base/UserModules/UserModuleThreadHelper.hh"

#include "TRACE/trace.h"

#include <future>
#include <limits>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace appframework {
/**
 * @brief FanOutUserModule sends data to multiple Buffers
 */
template <typename DATA_TYPE> class FanOutUserModule : public UserModule {
public:
  enum class FanOutMode {
    NotConfigured,
    Broadcast,
    RoundRobin,
    FirstAvailable
  };

  FanOutUserModule(
      std::shared_ptr<BufferOutput<DATA_TYPE>> inputBuffer,
      std::initializer_list<std::shared_ptr<BufferInput<DATA_TYPE>>>
          outputBuffers);

  /**
   * @brief Logs the reception of the command
   * @param cmd Command from DAQProcess
   */
  std::future<std::string> execute_command(std::string cmd) override;

  FanOutUserModule(const FanOutUserModule &) = delete;
  FanOutUserModule &operator=(const FanOutUserModule &) = delete;
  FanOutUserModule(FanOutUserModule &&) = delete;
  FanOutUserModule &operator=(FanOutUserModule &&) = delete;

private:
  // Commands
  std::string do_configure();
  std::string do_start();
  std::string do_stop();

  // Type traits handling. Yes, the "U" template parameter is actually
  // necessary, even though it's just an alias to this user module's
  // data type.

  template <typename U = DATA_TYPE>
  typename std::enable_if_t<!std::is_copy_constructible_v<U>>
  do_broadcast(DATA_TYPE &) const {
    throw std::runtime_error(
        "Broadcast mode cannot be used for non-copy-constructible types!");
  }
  template <typename U = DATA_TYPE>
  typename std::enable_if_t<std::is_copy_constructible_v<U>>
  do_broadcast(DATA_TYPE &data) const {
    for (auto &o : outputBuffers_) {
      auto starttime = std::chrono::steady_clock::now();
      o->push(data, bufferTimeout_);
      auto endtime = std::chrono::steady_clock::now();
      if (std::chrono::duration_cast<decltype(bufferTimeout_)>(
              endtime - starttime) > bufferTimeout_) {
        TLOG(TLVL_WARNING) << "Timeout occurred trying to broadcast data to "
                              "output buffer; data may be lost if it doesn't "
                              "make it into any other output buffers, either";
      }
    }
  }

  // Threading
  void do_work();
  UserModuleThreadHelper thread_;

  // Configuration
  FanOutMode mode_;
  std::chrono::milliseconds bufferTimeout_;

  std::shared_ptr<BufferOutput<DATA_TYPE>> inputBuffer_;
  std::list<std::shared_ptr<BufferInput<DATA_TYPE>>> outputBuffers_;
  size_t wait_interval_us_;
};
} // namespace appframework

#include "impl/FanOutUserModule.icc"

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_USERMODULES_FANOUTUSERMODULE_HH_
