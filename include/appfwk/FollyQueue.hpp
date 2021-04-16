#ifndef APPFWK_INCLUDE_APPFWK_FOLLYQUEUE_HPP_
#define APPFWK_INCLUDE_APPFWK_FOLLYQUEUE_HPP_

/**
 *
 * @file QueueI wrapper around folly::DynamicBoundedQueue
 *
 * The relevant types for users are FollySPSCQueue and FollyMPMCQueue,
 * which use the corresponding SPSC/MPMC specializations of
 * folly::DynamicBoundedQueue
 *
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/Queue.hpp"

#include "folly/concurrency/DynamicBoundedQueue.h"

#include <string>
#include <utility> // For std::move

namespace dunedaq::appfwk {

template<class T, template<typename, bool> class FollyQueueType>
class FollyQueue : public Queue<T>
{
public:
  using value_t = T;
  using duration_t = typename Queue<T>::duration_t;

  explicit FollyQueue(const std::string& name, size_t capacity)
    : Queue<T>(name, capacity)
    , m_queue(this->get_capacity())
  {}

  void do_pop(value_t& val, const duration_t& dur) override
  {
    if (!m_queue.try_dequeue_for(val, dur)) {
      throw QueueTimeoutExpired(
        ERS_HERE, this->get_name(), "pop", std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
    }
  }

  void do_push(value_t&& t, const duration_t& dur) override
  {
    if (!m_queue.try_enqueue_for(std::move(t), dur)) {
      throw QueueTimeoutExpired(
        ERS_HERE, this->get_name(), "push", std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
    }
  }

  // Delete the copy and move operations
  FollyQueue(const FollyQueue&) = delete;
  FollyQueue& operator=(const FollyQueue&) = delete;
  FollyQueue(FollyQueue&&) = delete;
  FollyQueue& operator=(FollyQueue&&) = delete;

private:
  // The boolean argument is `MayBlock`, where "block" appears to mean
  // "make a system call". With `MayBlock` set to false, the queue
  // just spin-waits, so we want true
  FollyQueueType<T, true> m_queue;
};

template<typename T>
using FollySPSCQueue = FollyQueue<T, folly::DSPSCQueue>;

template<typename T>
using FollyMPMCQueue = FollyQueue<T, folly::DMPMCQueue>;

} // namespace dunedaq::appfwk

#endif // APPFWK_INCLUDE_APPFWK_FOLLYQUEUE_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
