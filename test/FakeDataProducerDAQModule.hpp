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
#include "appfwk/ThreadHelper.hpp"
#include "appfwk/DAQSink.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace appfwk {
/**
 * @brief FakeDataProducerDAQModule creates vectors of ints and sends them
 * downstream
 */
class FakeDataProducerDAQModule : public DAQModule
{
public:
  /**
   * @brief FakeDataProducerDAQModule Constructor
   * @param name Instance name for this FakeDataProducerDAQModule instance
   */
  explicit FakeDataProducerDAQModule(const std::string& name);

  FakeDataProducerDAQModule(const FakeDataProducerDAQModule&) =
    delete; ///< FakeDataProducerDAQModule is not copy-constructible
  FakeDataProducerDAQModule& operator=(const FakeDataProducerDAQModule&) =
    delete; ///< FakeDataProducerDAQModule is not copy-assignable
  FakeDataProducerDAQModule(FakeDataProducerDAQModule&&) =
    delete; ///< FakeDataProducerDAQModule is not move-constructible
  FakeDataProducerDAQModule& operator=(FakeDataProducerDAQModule&&) =
    delete; ///< FakeDataProducerDAQModule is not move-assignable

  void init() override;
private:
  // Commands
  void do_configure(const std::vector<std::string>& args);
  void do_start(const std::vector<std::string>& args);
  void do_stop(const std::vector<std::string>& args);

  // Threading
  ThreadHelper thread_;
  void do_work();

  // Configuration
  std::unique_ptr<DAQSink<std::vector<int>>> outputQueue_;
  std::chrono::milliseconds queueTimeout_;
  size_t nIntsPerVector_;
  int starting_int_;
  int ending_int_;

  size_t wait_between_sends_ms_;
};
} // namespace appfwk

ERS_DECLARE_ISSUE_BASE(appfwk,
                       ProducerProgressUpdate,
                       appfwk::GeneralDAQModuleIssue,
                       name << ": " << message,
                       ERS_EMPTY,
                       ((std::string)name)((std::string)message))
} // namespace dunedaq

#endif // APP_FRAMEWORK_TEST_FAKEDATAPRODUCERDAQMODULE_HPP_
