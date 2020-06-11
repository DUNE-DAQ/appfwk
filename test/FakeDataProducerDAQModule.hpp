/**
 * @file FakeDataProducerDAQModule.hpp
 *
 * FakeDataProducerDAQModule is a simple DAQModule implementation that simply
 * logs the fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_TEST_FAKEDATAPRODUCERDAQMODULE_HPP_
#define APP_FRAMEWORK_TEST_FAKEDATAPRODUCERDAQMODULE_HPP_

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQModuleThreadHelper.hpp"
#include "appfwk/DAQSink.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq::appfwk {
/**
 * @brief FakeDataProducerDAQModule creates vectors of ints and sends them
 * downstream
 */
class FakeDataProducerDAQModule : public DAQModule {
public:
  /**
   * @brief FakeDataProducerDAQModule Constructor
   * @param name Instance name for this FakeDataProducerDAQModule instance
   */
  explicit FakeDataProducerDAQModule(const std::string &name);

  void execute_command(const std::string &cmd,
                       const std::vector<std::string> &args = {}) override;

  FakeDataProducerDAQModule(const FakeDataProducerDAQModule &) =
      delete; ///< FakeDataProducerDAQModule is not copy-constructible
  FakeDataProducerDAQModule &operator=(const FakeDataProducerDAQModule &) =
      delete; ///< FakeDataProducerDAQModule is not copy-assignable
  FakeDataProducerDAQModule(FakeDataProducerDAQModule &&) =
      delete; ///< FakeDataProducerDAQModule is not move-constructible
  FakeDataProducerDAQModule &operator=(FakeDataProducerDAQModule &&) =
      delete; ///< FakeDataProducerDAQModule is not move-assignable

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
} // namespace dunedaq::appfwk

#endif // APP_FRAMEWORK_TEST_FAKEDATAPRODUCERDAQMODULE_HPP_
