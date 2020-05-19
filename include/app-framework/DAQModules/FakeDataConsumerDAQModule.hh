/**
 * @file The FakeDataConsumerDAQModule class interface
 *
 * FakeDataConsumerDAQModule is a simple DAQModule implementation that simply
 * logs the fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQModuleS_FAKEDATACONSUMERDAQModule_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQModuleS_FAKEDATACONSUMERDAQModule_HH_

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/DAQModules/DAQModule.hh"
#include "app-framework-base/DAQModules/DAQModuleThreadHelper.hh"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace appframework {
/**
 * @brief FakeDataConsumerDAQModule creates vectors of ints and sends them
 * downstream
 */
class FakeDataConsumerDAQModule : public DAQModule {
public:
  explicit FakeDataConsumerDAQModule(
      std::shared_ptr<BufferOutput<std::vector<int>>> inputBuffer,
      std::string id = "");

  std::future<std::string> execute_command(std::string cmd) override;

  FakeDataConsumerDAQModule(const FakeDataConsumerDAQModule &) = delete;
  FakeDataConsumerDAQModule &
  operator=(const FakeDataConsumerDAQModule &) = delete;
  FakeDataConsumerDAQModule(FakeDataConsumerDAQModule &&) = delete;
  FakeDataConsumerDAQModule &operator=(FakeDataConsumerDAQModule &&) = delete;

private:
  // Commands
  std::string do_configure();
  std::string do_start();
  std::string do_stop();

  // Threading
  void do_work();
  DAQModuleThreadHelper thread_;

  // Configuration (for validation)
  size_t nIntsPerVector_;
  int starting_int_;
  int ending_int_;
  std::string id_;
  std::chrono::milliseconds bufferTimeout_;
  std::string getId() { return id_ != "" ? id_ + ": " : ""; }
  std::shared_ptr<BufferOutput<std::vector<int>>> inputBuffer_;
};
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQModuleS_FAKEDATACONSUMERDAQModule_HH_
