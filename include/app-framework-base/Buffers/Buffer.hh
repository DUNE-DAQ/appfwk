#ifndef app_framework_base_Buffers_Buffer_hh
#define app_framework_base_Buffers_Buffer_hh

#include <cstddef>
using std::size_t;

namespace appframework{

  template <class T>
  class Buffer {
    
  public:

    void Configure() = 0; ///called when configured in FSM
    
    bool isBounded() { return fBounded; }

    virtual size_t size() = 0; ///elements in buffer
    size_t sizeBytes() { return size()*sizeof(T); }
    
    virtual size_t capacity() = 0;
    size_t capacityBytes() { return capacity()*sizeof(T); }

    //basic element adding functions
    virtual size_t push(const T&) = 0; ///push one on
    virtual size_t pop(T&) = 0; ///pop one off


  protected:
    
    bool fBounded;  ///can size increase dynamically?

  };

}

#endif  // app_framework_base_Buffers_Buffer_hh
