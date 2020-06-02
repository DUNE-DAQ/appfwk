/**
 * @file The FakeDataProducerDAQModule class interface
 *
 * FakeDataProducerDAQModule is a simple DAQModule implementation that simply
 * logs the fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FAKEDATAPRODUCERDAQMODULE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FAKEDATAPRODUCERDAQMODULE_HH_

#include "app-framework/DAQModules/DAQModuleI.hh"
#include "app-framework/DAQModules/DAQModuleThreadHelper.hh"
#include "app-framework/DAQSink.hh"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace appframework {
/**
 * @brief FakeDataProducerDAQModule creates vectors of ints and sends them
 * downstream
 */
class FakeDataProducerDAQModule : public DAQModuleI
{
public:
  FakeDataProducerDAQModule(std::string name);

  void execute_command(const std::string& cmd,
                       const std::vector<std::string>& args = {}) override;

  FakeDataProducerDAQModule(const FakeDataProducerDAQModule&) = delete;
  FakeDataProducerDAQModule& operator=(const FakeDataProducerDAQModule&) =
    delete;
  FakeDataProducerDAQModule(FakeDataProducerDAQModule&&) = delete;
  FakeDataProducerDAQModule& operator=(FakeDataProducerDAQModule&&) = delete;

private:
  // Commands
  std::string do_configure();
  std::string do_start();
  std::string do_stop();

  // Threading
  DAQModuleThreadHelper thread_;
  void do_work();

  // Configuration
  std::unique_ptr<DAQSink<std::vector<int>>> outputQueue_;
  std::chrono::milliseconds queueTimeout_;
  size_t nIntsPerVector_;
  int starting_int_;
  int ending_int_;

  size_t wait_between_sends_ms_;
};
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_FAKEDATAPRODUCERDAQMODULE_HH_
