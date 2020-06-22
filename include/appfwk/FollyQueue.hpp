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

#include <utility> // For std::move
#include <string>

namespace dunedaq::appfwk {

template<class T, template <typename, bool> class FollyQueueType>
class FollyQueue : public Queue<T>
{
public:
  using value_type = T;
  using duration_type = typename Queue<T>::duration_type;

  explicit FollyQueue(const std::string& name, size_t capacity)
    : Queue<T>(name, capacity),
      fQueue(this->GetCapacity())
  {}

  bool can_pop() const noexcept override { return !fQueue.empty(); }
  bool pop( value_type & val, const duration_type& dur) override
  {
    return fQueue.try_dequeue_for(val, dur);
  }

  bool can_push() const noexcept override
  {
    return fQueue.size()<this->GetCapacity();
  }

  void push(value_type&& t, const duration_type& dur)  override
  {
    // Is the std::move actually necessary here?
    if (!fQueue.try_enqueue_for(std::move(t), dur)) {
      throw QueueTimeoutExpired(
        ERS_HERE, NamedObject::get_name(), "push", std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
    }
  }

   
  // Delete the copy and move operations
  FollyQueue(const FollyQueue&) = delete;
  FollyQueue& operator=(const FollyQueue&) = delete;
  FollyQueue(FollyQueue&&) = delete;
  FollyQueue& operator=(FollyQueue&&) = delete;

private:
  FollyQueueType<T, false> fQueue;
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
