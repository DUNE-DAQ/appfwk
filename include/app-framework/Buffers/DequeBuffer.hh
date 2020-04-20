#ifndef app_framework_Buffers_DequeBuffer_hh
#define app_framework_Buffers_DequeBuffer_hh

#include "app-framework-base/Buffers/Buffer.hh"

#include <unistd.h>
#include <deque>
#include <functional>
#include <mutex>

namespace appframework{

  template <class T>
  class DequeBuffer : public BufferInput<T>, BufferOutput<T> {
    
  public:

    DequeBuffer();

    void Configure();
    
    size_t size() { return fDeque.size(); }
    size_t capacity() { return fCapacity; }

    bool empty() { return size()==0; }
    bool full()  { return size()>=capacity(); }
    
    int push(const T&); ///push one on, return new size if successful, -1 if not
    T   pop();          ///pop one off, return object

  private:
    
    std::deque<T> fDeque;
    size_t        fCapacity;

    std::mutex fMutex;
    int fRetryTime;
    int fPushRetries;
    int fPopRetries;
  };

}


#endif //app_framework_Buffers_Buffer_hh
