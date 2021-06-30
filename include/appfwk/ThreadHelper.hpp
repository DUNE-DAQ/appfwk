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

#include "ers/ers.hpp"

#include <functional>
#include <future>
#include <list>
#include <memory>
#include <string>

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
/**
 * @brief An ERS Issue raised when a threading state error occurs
 */
ERS_DECLARE_ISSUE(appfwk,                              // Namespace
                  ThreadingIssue,                      // Issue Class Name
                  "Threading Issue detected: " << err, // Message
                  ((std::string)err))                  // Message parameters
// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {
/**
 * @brief ThreadHelper contains a thread which runs the do_work()
 * function
 *
 * ThreadHelper runs a given function in a std::thread and allows that
 * work function to be started and stopped via the
 * start_working_thread() and stop_working_thread() methods,
 * respectively. The work function takes a std::atomic<bool>&  which
 * indicates whether the thread should continue running, so a typical
 * implementation of a work function is:
 *
 * @code
 * void do_work(std::atomic<bool>& running_flag){
 *   while(running_flag.load()){
 *    // do something ...
 *   }
 * }
 * @endcode
 *
 * If your do_work function is a class member, you will need to wrap
 * it with std::bind or a lambda to bind the implicit 'this' argument,
 * eg
 *
 * @code
 * class MyClass {
 * public:
 *   MyClass()
 *     : helper_(std::bind(MyClass::do_work, this, std::placeholders::_1))
 *   {}
 *   void do_work(std::atomic<bool>& running_flag) { ... }
 *   ThreadHelper helper_;
 * };
 * @endcode
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
  explicit ThreadHelper(std::function<void(std::atomic<bool>&)> do_work)
    : m_thread_running(false)
    , m_working_thread(nullptr)
    , m_do_work(do_work)
  {}

  /**
   * @brief Start the working thread (which executes the do_work() function)
   * @throws ThreadingIssue if the thread is already running
   */
  void start_working_thread(const std::string& name = "noname")
  {
    if (thread_running()) {
      throw ThreadingIssue(ERS_HERE,
                           "Attempted to start working thread "
                           "when it is already running!");
    }
    m_thread_running = true;
    m_working_thread.reset(new std::thread([&] { m_do_work(std::ref(m_thread_running)); }));
    auto handle = m_working_thread->native_handle();
    auto rc = pthread_setname_np(handle, name.c_str());
    if (rc != 0) {
      std::ostringstream s;
      s << "The name " << name << " provided for the thread is too long.";
      ers::warning(ThreadingIssue(ERS_HERE, s.str()));
    }
  }
  /**
   * @brief Stop the working thread
   * @throws ThreadingIssue If the thread has not yet been started
   * @throws ThreadingIssue If the thread is not in the joinable state
   * @throws ThreadingIssue If an exception occurs during thread join
   */
  void stop_working_thread()
  {
    if (!thread_running()) {
      throw ThreadingIssue(ERS_HERE,
                           "Attempted to stop working thread "
                           "when it is not running!");
    }
    m_thread_running = false;

    if (m_working_thread->joinable()) {
      try {
        m_working_thread->join();
      } catch (std::system_error const& e) {
        throw ThreadingIssue(ERS_HERE, std::string("Error while joining thread, ") + e.what());
      }
    } else {
      throw ThreadingIssue(ERS_HERE, "Thread not in joinable state during working thread stop!");
    }
  }

  /**
   * @brief Determine if the thread is currently running
   * @return Whether the thread is currently running
   */
  bool thread_running() const { return m_thread_running.load(); }

  ThreadHelper(const ThreadHelper&) = delete;            ///< ThreadHelper is not copy-constructible
  ThreadHelper& operator=(const ThreadHelper&) = delete; ///< ThreadHelper is not copy-assginable
  ThreadHelper(ThreadHelper&&) = delete;                 ///< ThreadHelper is not move-constructible
  ThreadHelper& operator=(ThreadHelper&&) = delete;      ///< ThreadHelper is not move-assignable

private:
  std::atomic<bool> m_thread_running;
  std::unique_ptr<std::thread> m_working_thread;
  std::function<void(std::atomic<bool>&)> m_do_work;
};
} // namespace appfwk

} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_THREADHELPER_HPP_
