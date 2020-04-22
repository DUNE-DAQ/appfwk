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

#include <unistd.h>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>

namespace appframework{

  template <class T>
  class DequeBuffer : public BufferInput<T>, BufferOutput<T> {
  public:
    DequeBuffer();

    void Configure();
    
    size_t size() { return fSize.load() ; }
    size_t capacity() { return fCapacity; }

    bool empty() { return size()==0; }
    bool full()  { return size()>=capacity(); }
    
    int push(T&&);  /// push one on, return new size if successful, -1 if not
    T   pop();            ///pop one off, return object

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

  if(this->attributes().isBounded){
    fCapacity = 1000;
    } else {
    fCapacity = fDeque.max_size();
  }

  this->fPushTimeout_ms = 1e6;
  this->fPopTimeout_ms  = 1e7;
  fRetryTime_ms         = 100;

  fPushRetries = this->fPushTimeout_ms/fRetryTime_ms;
  fPopRetries  = this->fPopTimeout_ms/fRetryTime_ms;
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
