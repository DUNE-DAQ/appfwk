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

}  // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_BUFFERS_DEQUEBUFFER_HH_
