/**
 * @file FakeDataConsumerDAQModule.hpp
 *
 * FakeDataConsumerDAQModule is a simple DAQModule implementation that receives vectors of integers produced by
 * FakeDataProducerDAQModule and checks that they are consistent.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_TEST_FAKEDATACONSUMERDAQMODULE_HPP_
#define APPFWK_TEST_FAKEDATACONSUMERDAQMODULE_HPP_

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSource.hpp"
#include "appfwk/ThreadHelper.hpp"

// Our command data structures
#include "appfwk/fakedataconsumerdaqmodule/Structs.hpp"

#include <ers/Issue.h>

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace appfwk {
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

  void init(const nlohmann::json& ) override;

private:
  // Commands
  void do_configure(const data_t& data);
  void do_start(const data_t& data);
  void do_stop(const data_t& data);

  // Threading
  void do_work(std::atomic<bool>& running_flag);
  ThreadHelper thread_;

  // Configuration (for validation)
  fakedataconsumerdaqmodule::Conf cfg_;
  // size_t nIntsPerVector_ = 999;
  // int starting_int_ = -999;
  // int ending_int_ = -999;
  std::chrono::milliseconds queueTimeout_;
  std::unique_ptr<DAQSource<std::vector<int>>> inputQueue_;
};

} // namespace appfwk
ERS_DECLARE_ISSUE_BASE(appfwk,
                       ConsumerErrorDetected,
                       appfwk::GeneralDAQModuleIssue,

                       "Error in received vector " << counter << " at position " << position << ": Expected "
                                                   << expected << ", received " << received,
                       ((std::string)name),
                       ((int)counter)((int)position)((int)expected)((int)received))
ERS_DECLARE_ISSUE_BASE(appfwk,
                       ConsumerProgressUpdate,
                       appfwk::GeneralDAQModuleIssue,
                       message,
                       ((std::string)name),
                       ((std::string)message))
} // namespace dunedaq

#endif // APPFWK_TEST_FAKEDATACONSUMERDAQMODULE_HPP_
