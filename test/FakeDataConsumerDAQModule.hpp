/**
 * @file FakeDataConsumerDAQModule.hpp
 *
 * FakeDataConsumerDAQModule is a simple DAQModule implementation that simply
 * logs the fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_TEST_FAKEDATACONSUMERDAQMODULE_HH_
#define APP_FRAMEWORK_TEST_FAKEDATACONSUMERDAQMODULE_HH_

#include "app-framework/DAQModule.hpp"
#include "app-framework/DAQModuleThreadHelper.hpp"
#include "app-framework/DAQSource.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace appframework {
/**
 * @brief FakeDataConsumerDAQModule creates vectors of ints and sends them
 * downstream
 */
class FakeDataConsumerDAQModule : public DAQModule
{
public:
  /**
   * @brief FakeDataConsumerDAQModule Constructor
   * @param name Instance name for this FakeDataConsumerDAQModule instance
   */
  explicit FakeDataConsumerDAQModule(const std::string & name);

  void execute_command(const std::string& cmd,
                       const std::vector<std::string>& args = {}) override;

  FakeDataConsumerDAQModule(const FakeDataConsumerDAQModule&) = delete; ///< FakeDataConsumerDAQModule is not copy-constructible
  FakeDataConsumerDAQModule& operator=(const FakeDataConsumerDAQModule&) =
    delete;///< FakeDataConsumerDAQModule is not copy-assignable
  FakeDataConsumerDAQModule(FakeDataConsumerDAQModule&&) = delete; ///< FakeDataConsumerDAQModule is not move-constructible
  FakeDataConsumerDAQModule& operator=(FakeDataConsumerDAQModule&&) = delete; ///< FakeDataConsumerDAQModule is not move-assignable

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

#endif // APP_FRAMEWORK_TEST_FAKEDATACONSUMERDAQMODULE_HH_
