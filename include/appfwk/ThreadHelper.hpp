/**
 * @file ThreadHelper.hpp ThreadHelper class declarations
 *
 * ThreadHelper defines a std::thread which runs the do_work()
 * function as well as methods to start and stop that thread.
 * This file is intended to help reduce code duplication for the common
 * task of starting and stopping threads. As in artdaq, std::thread may
 * be replaced by boost::thread to allow setting the stack size at a
 * later date if that functionality is found to be necessary.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_THREADHELPER_HPP_
#define APPFWK_INCLUDE_APPFWK_THREADHELPER_HPP_

#include <ers/Issue.h>

#include <functional>
#include <future>
#include <list>
#include <memory>
#include <string>

namespace dunedaq {

/**
 * @brief An ERS Issue raised when a threading state error occurs
 */
ERS_DECLARE_ISSUE(appfwk, // Namespace
                  ThreadingIssue, // Issue Class Name
                  "Threading Issue detected: " << err, // Message
                  ((std::string)err)) // Message parameters

namespace appfwk {
/**
 * @brief ThreadHelper contains a thread which runs the do_work()
 * function
 */
class ThreadHelper
{
public:
  /**
   * @brief ThreadHelper Constructor
   * @param do_work Function to be executed in the thread
   *
   * This constructor sets the defaults for the thread control variables
   */
  explicit ThreadHelper(std::function<void()> do_work)
    : thread_running_(false)
    , working_thread_(nullptr)
    , do_work_(do_work)
  {}

  /**
   * @brief Start the working thread (which executes the do_work() function)
   * @throws ThreadingIssue if the thread is already running
   */
  void start_working_thread_()
  {
    if (thread_running()) {
      throw ThreadingIssue(ERS_HERE, "Attempted to start working thread "
                           "when it is already running!");
    }
    thread_running_ = true;
    working_thread_.reset(new std::thread([&] { do_work_(); }));
  }
  /**
   * @brief Stop the working thread
   * @throws ThreadingIssue If the thread has not yet been started
   * @throws ThreadingIssue If the thread is not in the joinable state
   * @throws ThreadingIssue If an exception occurs during thread join
   */
  void stop_working_thread_()
  {
    if (!thread_running()) {
      throw ThreadingIssue(ERS_HERE,
                           "Attempted to stop working thread "
                               "when it is not running!");
    }
    thread_running_ = false;

    if (working_thread_->joinable()) {
      try {
        working_thread_->join();
      } catch (std::system_error e) {
        throw ThreadingIssue(ERS_HERE, std::string("Error while joining thread, ") + e.what());
      }
    } else {
      throw ThreadingIssue(ERS_HERE,
                           "Thread not in joinable state during working thread stop!");
    }
  }

  /**
   * @brief Determine if the thread is currently running
   * @return Whether the thread is currently running
   */
  bool thread_running() const { return thread_running_.load(); }

  ThreadHelper(const ThreadHelper&) = delete;            ///< ThreadHelper is not copy-constructible
  ThreadHelper& operator=(const ThreadHelper&) = delete; ///< ThreadHelper is not copy-assginable
  ThreadHelper(ThreadHelper&&) = delete;                 ///< ThreadHelper is not move-constructible
  ThreadHelper& operator=(ThreadHelper&&) = delete;      ///< ThreadHelper is not move-assignable

private:
  std::atomic<bool> thread_running_;
  std::unique_ptr<std::thread> working_thread_;
  std::function<void()> do_work_;
};
} // namespace dunedaq::appfwk


} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_THREADHELPER_HPP_
