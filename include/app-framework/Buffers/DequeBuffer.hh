#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_

/**
 *
 * @file A deque
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */


#include "app-framework-base/Buffers/Buffer.hh"

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <utility>
#include <unistd.h>

namespace appframework{

  template <class T>
  class DequeBuffer : public BufferInput<T>, public BufferOutput<T> {
  public:
    DequeBuffer();

    void Configure();
    
    size_t size() const noexcept { return fSize.load() ; }
    size_t capacity() const noexcept override { return fCapacity; }

    bool empty() const noexcept override { return size()==0; }
    bool full() const noexcept override { return size()>=capacity(); }
    
    int push(T&&);  /// push one on, return new size if successful, -1 if not
    T   pop();            ///pop one off, return object

    // Delete the copy and move operations since member data instances
    // of std::mutex and std::atomic aren't copyable or movable

    DequeBuffer(const DequeBuffer& ) = delete;
    DequeBuffer& operator=(const DequeBuffer& ) = delete;
    DequeBuffer(DequeBuffer&& ) = delete;
    DequeBuffer& operator=(DequeBuffer&& ) = delete;

  private:
    std::deque<T>       fDeque;
    std::atomic<size_t> fSize = 0 ;
    size_t              fCapacity;

    std::mutex fMutex;
    size_t fRetryTime_ms;
    size_t fPushRetries;
    size_t fPopRetries;
  };


template <class T>
DequeBuffer<T>::DequeBuffer(){
  Configure();
}

template <class T>
void DequeBuffer<T>::Configure(){

  this->fAttributes.isBounded = true;
  this->fAttributes.isSearchable = false;
  
  fDeque = std::deque<T>();
  fSize = 0 ;

  if(this->fAttributes.isBounded){
    fCapacity = 1000;
    } else {
    fCapacity = fDeque.max_size();
  }

  this->set_push_timeout( 1e6 );
  this->set_pop_timeout( 1e7 );
  fRetryTime_ms         = 100;

  fPushRetries = this->get_push_timeout()/fRetryTime_ms;
  fPopRetries  = this->get_pop_timeout()/fRetryTime_ms;
}

template <class T>
int DequeBuffer<T>::push(T&& obj) {
  int n_retries=0;
  while(full()){
    if(n_retries==fPushRetries){
      //could throw, but here choose to return -1
      //throw std::runtime_error("Reached Max Push Retries in DequeBuffer");
      return -1;
    }
    ++n_retries;
    usleep(fRetryTime_ms);
  }
  
  if(full()) throw std::runtime_error("Full after check? Another thread accessing?");
  std::lock_guard<std::mutex> lck(fMutex);
    fDeque.emplace_back(obj);
  fSize ++ ;
  return size();
}

template <class T>
T DequeBuffer<T>::pop(){
  int n_retries=0;
  while(empty()){
    if(n_retries==fPopRetries){
      throw std::runtime_error("No Data: Reached Max Pop Retries in DequeBuffer");
      //return -1;
    }
    ++n_retries;
    usleep(fRetryTime_ms);
  }

  if(empty()) throw std::runtime_error("Empty after check? Another thread accessing?");

  std::lock_guard<std::mutex> lck(fMutex);
  T obj(std::move(fDeque.front()));
  fDeque.pop_front();
    fSize--;
  return obj;
}

}  // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_
