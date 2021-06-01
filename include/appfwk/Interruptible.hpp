/**
 * @file Interruptible.hpp Interface for things that can be interrupted
 *
 * This interface defines the concept of an object that can be "Interrupted".
 * It is up to the implementation to decide how to use interruptible_wait to
 * have a valid concretization of the concept of "Interrupt"
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_INTERRUPTIBLE_HPP_
#define APPFWK_INCLUDE_APPFWK_INTERRUPTIBLE_HPP_

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace dunedaq {
namespace appfwk {
class Interruptible
{
public:
  Interruptible()
    : m_wait_cv()
    , m_wait_mutex()
  {}

  /**
   * @brief Send a notification that an interrupt is requested.
   *
   * This function is virtual to allow classes which inherit from Interruptible to implement
   * custom behavior (such as calling interrupt on Interruptible object members)
   */
  virtual void interrupt() { interrupt_self(); }

  /**
   * @brief Send a notification to all threads currently in a call to interruptible_wait
   *
   * Note that there is minimal penalty for calling interrupt_self() when no threads are waiting, so it can
   * be called multiple times in succession, for example by a "stop" command handler and then by
   * execute_command.
   */
  void interrupt_self()
  {
    std::unique_lock<std::mutex> wait_lock(m_wait_mutex);
    m_wait_cv.notify_all();
  }

protected:
  /**
   * @brief Sleep for the given amount of time while wait_condition evaluates to false
   * @param wait_duration The amount of time to sleep for
   * @param wait_condition An atomic bool which indicates via the direction parameter if the sleep
   * should be continued (default direction is false, so a false wait_condition indicates that the
   * sleep should continue)
   * @param direction Indicates which value of wait_condition should indicate if the sleep should
   * continue
   * @returns The result of wait_condition after the sleep
   *
   * Note that calling interrupt() will cause an evaluation of wait_condition, and if the condition
   * still indicates "sleep", the sleep will continue. Therefore, interrupt() should be called only
   * after the state of the DAQModule has been changed.
   */
  bool interruptible_wait(std::chrono::microseconds wait_duration,
                          std::atomic<bool>& wait_condition,
                          bool direction = false)
  {
    std::unique_lock<std::mutex> wait_lock(m_wait_mutex);

    // Wait Predicate should return false to continue wait
    return m_wait_cv.wait_for(wait_lock, wait_duration, [&]() { return wait_condition.load() != direction; });
  }

private:
  // For interruptible waits
  std::condition_variable m_wait_cv;
  std::mutex m_wait_mutex;
};
} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_INTERRUPTIBLE_HPP_