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

#include <memory>  // for unique_ptr
#include <utility> // For std::move
#include <string>

namespace dunedaq::appfwk {

template<class T, template <typename, bool> class FollyQueueType>
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
  value_type pop( const duration_type& dur) override
  {
    std::unique_ptr<T> sp ;  
    if ( fQueue.try_dequeue_for( sp, dur) ) 
      return std::move(*sp) ;
    else {
      std::stringstream errmsg;
      errmsg
	<< "In FollyQueue::pop_wait_for: unable to pop since queue is "
	"empty (timeout period was "
	<< std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()
	<< " milliseconds)";
      throw std::runtime_error(errmsg.str());
    }
  }

  bool can_push() const noexcept override
  {
    return fQueue.size()<fMaxSize;
  }

  void push(value_type&& t, const duration_type& dur)  override
  {
    // Is the std::move actually necessary here?
    if(!fQueue.try_enqueue_for( std::move( std::unique_ptr<T>( new T (std::move(t) ) ) ), dur)){
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
  FollyQueueType<std::unique_ptr<T>, false> fQueue;
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
