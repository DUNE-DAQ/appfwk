#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_FOLLYQUEUE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_FOLLYQUEUE_HH_

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

namespace dunedaq::appfwk {

// TODO: I'd really like FollyQueueType to have a `T` in it somewhere,
// so we can't instantiate this class with a folly queue containing a
// type other than T, but I can't work out the necessary C++
// incantation, so this way it is, for now
template<class T, class FollyQueueType>
class FollyQueue : public Queue<T>
{
public:
  using value_type = T;
  using duration_type = typename Queue<T>::duration_type;

  explicit FollyQueue(const std::string& name)
    : Queue<T>(name),
      fMaxSize(0),
      fQueue(fMaxSize)
  {}

  bool can_pop() const noexcept override { return !fQueue.empty(); }
  bool pop( value_type & val, const duration_type& dur) override
  {
    return fQueue.try_dequeue_for(val, dur);
  }

  bool can_push() const noexcept override
  {
    return fQueue.size()<fMaxSize;
  }

  void push(value_type&& t, const duration_type& dur)  override
  {
    // Is the std::move actually necessary here?
    if(!fQueue.try_enqueue_for(std::move(t), dur)){
      throw std::runtime_error("In FollyQueue::push: unable to push since queue is full");
    }
  }

  /**
   * @brief Set the size of the FollyQueue
   * @param sz Maximum size for the FollyQueue
  */
  void SetSize(size_t sz) { fMaxSize = sz; fQueue.reset_capacity(fMaxSize); }

   
  // Delete the copy and move operations
  FollyQueue(const FollyQueue&) = delete;
  FollyQueue& operator=(const FollyQueue&) = delete;
  FollyQueue(FollyQueue&&) = delete;
  FollyQueue& operator=(FollyQueue&&) = delete;

private:
  size_t fMaxSize;
  FollyQueueType fQueue;
};

template<typename T>
using FollySPSCQueue = FollyQueue<T, folly::DSPSCQueue<T, false>>;

template<typename T>
using FollyMPMCQueue = FollyQueue<T, folly::DMPMCQueue<T, false>>;

} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUES_FOLLYQUEUE_HH_


// Local Variables:
// c-basic-offset: 2
// End:
