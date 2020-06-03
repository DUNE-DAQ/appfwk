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

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FAKEDATACONSUMERDAQMODULE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FAKEDATACONSUMERDAQMODULE_HH_

#include "app-framework/DAQModules/DAQModuleI.hh"
#include "app-framework/DAQModules/DAQModuleThreadHelper.hh"
#include "app-framework/DAQSource.hh"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace appframework {
/**
 * @brief FakeDataConsumerDAQModule creates vectors of ints and sends them
 * downstream
 */
class FakeDataConsumerDAQModule : public DAQModuleI
{
public:
  FakeDataConsumerDAQModule(std::string name);

  void execute_command(const std::string& cmd,
                       const std::vector<std::string>& args = {}) override;

  FakeDataConsumerDAQModule(const FakeDataConsumerDAQModule&) = delete;
  FakeDataConsumerDAQModule& operator=(const FakeDataConsumerDAQModule&) =
    delete;
  FakeDataConsumerDAQModule(FakeDataConsumerDAQModule&&) = delete;
  FakeDataConsumerDAQModule& operator=(FakeDataConsumerDAQModule&&) = delete;

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
  std::chrono::milliseconds queueTimeout_;
  std::unique_ptr<DAQSource<std::vector<int>>> inputQueue_;
};
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FAKEDATACONSUMERDAQMODULE_HH_
