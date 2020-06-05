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

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUE_HPP_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUE_HPP_

#include "app-framework/NamedObject.hpp"

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace appframework {

/**
 * @brief Implementations of the Queue class are responsible for relaying data
 * between DAQModules within a DAQ Application
 *
 * Note that while the Queue class itself is not templated on a data type (so
 * it can be included in generic containers), all implementations should be.
 */
template<class T>
class Queue : public NamedObject
{
public:
  using value_type = T; ///< Type stored in the Queue
  using duration_type = std::chrono::milliseconds; ///< Base duration type for timeouts

  /**
   * @brief Queue Constructor
   * @param name Name of the Queue instance
  */
  explicit Queue(const std::string & name)
    : NamedObject(name)
  {}

  /**
   * @brief Push a value onto the Queue.
   * @param val Value to push (rvalue)
   * @param timeout Timeout for the push operation. 
   *
   * This is a pure virtual function.
   * If push takes longer than the timeout, implementations should throw an exception.
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
   * @param timeout Timeout for the pop operation
   * @return Value popped from the Queue
   *
   * This is a pure virtual function
   * If pop takes longer than the timeout, implementations should throw an exception
  */
  virtual T pop(const duration_type& timeout) = 0;

  /**
   * @brief Determine whether the Queue may be popped from
   * @return True if the queue is not empty, false if it is
   *
   * This is a pure virtual function 
  */
  virtual bool can_pop() const noexcept = 0;

private:
  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue(Queue&&) = default;
  Queue& operator=(Queue&&) = default;
};

} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUE_HPP_
