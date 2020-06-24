
#include "ers/ers.h"

namespace dunedaq::appfwk {

template<class T>
StdDeQueue<T>::StdDeQueue(const std::string& name, size_t capacity)
  : Queue<T>(name, capacity)
  , fDeque()
  , fSize(0)
{
  assert(fDeque.max_size() > this->GetCapacity());
}

template<class T>
void
StdDeQueue<T>::push(value_type&& object_to_push, const duration_type& timeout)
{

  auto starttime = std::chrono::steady_clock::now();
  std::unique_lock<std::mutex> lk(fMutex, std::defer_lock);

  this->try_lock_for(lk, timeout);

  auto time_to_wait_for_space = (starttime + timeout) - std::chrono::steady_clock::now();

  if (time_to_wait_for_space.count() > 0) {
    fNoLongerFull.wait_for(lk, time_to_wait_for_space, [&]() { return this->can_push(); });
  }

  if (this->can_push()) {
    fDeque.push_back(std::move(object_to_push));
    fSize++;
    fNoLongerEmpty.notify_one();
  } else {
    throw QueueTimeoutExpired(ERS_HERE,
                              this->get_name(),
                              "push",
                              std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
  }
}

template<class T>
void
StdDeQueue<T>::pop(T& val, const duration_type& timeout)
{

  auto starttime = std::chrono::steady_clock::now();
  std::unique_lock<std::mutex> lk(fMutex, std::defer_lock);

  this->try_lock_for(lk, timeout);

  auto time_to_wait_for_data = (starttime + timeout) - std::chrono::steady_clock::now();

  if (time_to_wait_for_data.count() > 0) {
    fNoLongerEmpty.wait_for(lk, time_to_wait_for_data, [&]() { return this->can_pop(); });
  }

  if (this->can_pop()) {
    val = std::move(fDeque.front());
    fDeque.pop_front();
    fSize--;
    fNoLongerFull.notify_one();
  } else {
    throw QueueTimeoutExpired(
      ERS_HERE, this->get_name(), "pop", std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
  }
}

// This try_lock_for() function was written because while objects of
// type std::timed_mutex have their own try_lock_for functions, the
// std::condition_variable::wait_for functions used in this class's push
// and pop operations require an std::mutex

template<class T>
void
StdDeQueue<T>::try_lock_for(std::unique_lock<std::mutex>& lk, const duration_type& timeout)
{
  assert(!lk.owns_lock());

  auto starttime = std::chrono::steady_clock::now();
  lk.try_lock();

  if (!lk.owns_lock() && timeout.count() > 0) {

    int approximate_number_of_retries = 5;
    duration_type pause_between_tries = duration_type(timeout.count() / approximate_number_of_retries);

    while (std::chrono::steady_clock::now() < starttime + timeout) {
      std::this_thread::sleep_for(pause_between_tries);
      lk.try_lock();
      if (lk.owns_lock()) {
        break;
      }
    }
  }

  if (!lk.owns_lock()) {
    throw QueueTimeoutExpired(ERS_HERE,
                              this->get_name(),
                              "lock mutex",
                              std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
  }
}

} // namespace dunedaq::appfwk
