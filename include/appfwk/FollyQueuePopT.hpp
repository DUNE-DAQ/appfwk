#ifndef APPFWK_INCLUDE_APPFWK_FOLLYQUEUE_POPT_HPP_
#define APPFWK_INCLUDE_APPFWK_FOLLYQUEUE_POPT_HPP_

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
class FollyQueuePopT : public Queue<T>
{
public:
  using value_type = T;
  using duration_type = typename Queue<T>::duration_type;

  explicit FollyQueuePopT(const std::string& name)
    : Queue<T>(name),
      fMaxSize(0),
      fQueue(fMaxSize)
  {}

  bool can_pop() const noexcept override { return !fQueue.empty(); }

  void pop( value_type t, const duration_type& dur) {
    if ( !fQueue.try_dequeue_for( t, dur) ) {
      std::stringstream errmsg;
      errmsg
	<< "In FollyQueuePopT::pop: unable to pop since queue is "
	"empty (timeout period was "
	<< std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()
	<< " milliseconds)";
      throw std::runtime_error(errmsg.str());
    }
  }

  value_type pop( const duration_type& dur) override
  {
    // Don't call T pop on this class, call pop T
    assert(false);
  }
   

  bool can_push() const noexcept override
  {
    return fQueue.size()<fMaxSize;
  }

  void push(value_type&& t, const duration_type& dur)  override
  {
    if(!fQueue.try_enqueue_for( t, dur)){
      throw std::runtime_error("In FollyQueuePopT::push: unable to push since queue is full");
    }
  }

  /**
   * @brief Set the size of the FollyQueuePopT
   * @param sz Maximum size for the FollyQueuePopT
  */
  void SetSize(size_t sz) { fMaxSize = sz; fQueue.reset_capacity(fMaxSize); }

   
  // Delete the copy and move operations
  FollyQueuePopT(const FollyQueuePopT&) = delete;
  FollyQueuePopT& operator=(const FollyQueuePopT&) = delete;
  FollyQueuePopT(FollyQueuePopT&&) = delete;
  FollyQueuePopT& operator=(FollyQueuePopT&&) = delete;

private:
  size_t fMaxSize;
  FollyQueueType<T, false> fQueue;
};

template<typename T>
using FollySPSCQueuePopT = FollyQueuePopT<T, folly::DSPSCQueue>;

template<typename T>
using FollyMPMCQueuePopT = FollyQueuePopT<T, folly::DMPMCQueue>;

} // namespace dunedaq::appfwk

#endif // APPFWK_INCLUDE_APPFWK_FOLLYQUEUE_POPT_HPP_


// Local Variables:
// c-basic-offset: 2
// End:
