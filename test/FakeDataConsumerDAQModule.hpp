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

#include "appfwk/DAQModule.hpp"
#include "appfwk/ThreadHelper.hpp"
#include "appfwk/DAQSource.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq::appfwk {
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
  explicit FakeDataConsumerDAQModule(const std::string& name);

  FakeDataConsumerDAQModule(const FakeDataConsumerDAQModule&) =
    delete; ///< FakeDataConsumerDAQModule is not copy-constructible
  FakeDataConsumerDAQModule& operator=(const FakeDataConsumerDAQModule&) =
    delete; ///< FakeDataConsumerDAQModule is not copy-assignable
  FakeDataConsumerDAQModule(FakeDataConsumerDAQModule&&) =
    delete; ///< FakeDataConsumerDAQModule is not move-constructible
  FakeDataConsumerDAQModule& operator=(FakeDataConsumerDAQModule&&) =
    delete; ///< FakeDataConsumerDAQModule is not move-assignable

private:
  // Commands
  void do_configure(const std::vector<std::string>& args);
  void do_start(const std::vector<std::string>& args);
  void do_stop(const std::vector<std::string>& args);

  // Threading
  void do_work();
  ThreadHelper thread_;

  // Configuration (for validation)
  size_t nIntsPerVector_;
  int starting_int_;
  int ending_int_;
  std::chrono::milliseconds queueTimeout_;
  std::unique_ptr<DAQSource<std::vector<int>>> inputQueue_;
};
} // namespace dunedaq::appfwk

#endif // APP_FRAMEWORK_TEST_FAKEDATACONSUMERDAQMODULE_HH_
