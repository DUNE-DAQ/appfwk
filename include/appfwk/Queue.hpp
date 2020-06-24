/**
 * @file Queue.hpp
 *
 * This is the interface for Queue objects which connect DAQModules. Queues
 * are exposed to DAQModules via the DAQSource and DAQSink classes, and should
 * not be handled directly. Queues are registered with QueueRegistry for
 * retrieval by DAQSink and DAQSource instances.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_QUEUE_HPP_
#define APPFWK_INCLUDE_APPFWK_QUEUE_HPP_

#include "appfwk/NamedObject.hpp"

#include <ers/Issue.h>

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace appfwk {

/**
 * @brief Implementations of the Queue class are responsible for relaying data
 * between DAQModules within a DAQ Application
 *
 * Note that while the Queue class itself is not templated on a data type (so
 * it can be included in generic containers), all implementations should be.
 */
template<class T>
class Queue : public NamedObjectImpl
{
public:
  using value_type = T;                            ///< Type stored in the Queue
  using duration_type = std::chrono::milliseconds; ///< Base duration type for timeouts

  /**
   * @brief Queue Constructor
   * @param name Name of the Queue instance
   */
  explicit Queue(const std::string& name, size_t capacity)
    : NamedObjectImpl(name)
    , capacity_(capacity)
  {}

  /**
   * @brief Push a value onto the Queue.
   * @param val Value to push (rvalue)
   * @param timeout Timeout for the push operation.
   *
   * This is a pure virtual function.
   * If push takes longer than the timeout, implementations should throw an
   * exception.
   */
  virtual void push(T&& val, const duration_type& timeout) = 0;

  /**
   * @brief Determine whether the Queue may be pushed onto
   * @return True if the queue is not full, false if it is
   *
   * This is a pure virtual function
   */
  virtual bool can_push() const noexcept = 0;

  /**
   * @brief Pop the first value off of the queue
   * @param val Reference to the value that is popped from the queue
   * @param timeout Timeout for the pop operation
   *
   * This is a pure virtual function
   * If pop takes longer than the timeout, implementations should throw an
   * exception
   */
  virtual void pop(T& val, const duration_type& timeout) = 0;

  /**
   * @brief Determine whether the Queue may be popped from
   * @return True if the queue is not empty, false if it is
   *
   * This is a pure virtual function
   */
  virtual bool can_pop() const noexcept = 0;

protected:
  size_t GetCapacity() const { return capacity_; }

private:
  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue(Queue&&) = default;
  Queue& operator=(Queue&&) = default;

  size_t capacity_;
};

} // namespace appfwk
/**
 * @brief QueueTimeoutExpired ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,              // namespace
                  QueueTimeoutExpired, // issue class name
                  name << ": Unable to " << func_name << " within timeout period (timeout period was " << timeout
                       << " milliseconds)", // message
                  ((std::string)name)((std::string)func_name)((int)timeout))
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_QUEUE_HPP_
