/**
 * @file The DAQModuleThreadHelper class interface
 *
 * DAQModuleThreadHelper defines a std::thread which runs the do_work()
 * function as well as methods to start and stop that thread.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULETHREADHELPER_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULETHREADHELPER_HH_

#include <functional>
#include <future>
#include <list>
#include <memory>
#include <string>

namespace appframework {
/**
 * @brief DAQModuleThreadHelper contains a thread which runs the do_work()
 * function
 */
class DAQModuleThreadHelper
{
public:
  /**
   * @brief DAQModuleThreadHelper Constructor
   *
   * This constructor sets the defaults for the thread control variables
   */
  explicit DAQModuleThreadHelper(std::function<void()> do_work)
    : thread_running_(false)
    , working_thread_(nullptr)
    , do_work_(do_work)
  {}

  /**
   * @brief Start the working thread (which executes the do_work() function)
   * @throws std::runtime_error if the thread is already running
   */
  void start_working_thread_()
  {
    if (thread_running()) {
      throw std::runtime_error("Attempted to start DAQModule working thread "
                               "when it is already running!");
    }
    thread_running_ = true;
    working_thread_.reset(new std::thread([&] { do_work_(); }));
  }
  /**
   * @brief Stop the working thread
   * @throws std::runtime_error If the thread has not yet been started
   * @throws std::runtime_error If the thread is not in the joinable state
   */
  void stop_working_thread_()
  {
    if (!thread_running()) {
      throw std::runtime_error("Attempted to stop DAQModule working thread "
                               "when it is not running!");
    }
    thread_running_ = false;

    if (working_thread_->joinable()) {
      working_thread_->join();
    } else {
      throw std::runtime_error("Thread not in joinable state during DAQModule "
                               "working thread stop!");
    }
  }

  bool thread_running() const { return thread_running_.load(); }

  DAQModuleThreadHelper(const DAQModuleThreadHelper&) = delete;
  DAQModuleThreadHelper& operator=(const DAQModuleThreadHelper&) = delete;
  DAQModuleThreadHelper(DAQModuleThreadHelper&&) = delete;
  DAQModuleThreadHelper& operator=(DAQModuleThreadHelper&&) = delete;

private:
  std::atomic<bool> thread_running_;
  std::unique_ptr<std::thread> working_thread_;
  std::function<void()> do_work_;
};
} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULETHREADHELPER_HH_
