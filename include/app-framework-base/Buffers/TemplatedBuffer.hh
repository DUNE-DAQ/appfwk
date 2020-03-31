#ifndef app_framework_base_Buffers_TemplatedBuffer_hh
#define app_framework_base_Buffers_TemplatedBuffer_hh

#include "Buffer.hh"

#include <cstddef>
using std::size_t;

namespace appframework{

  template <class T>
  class TemplatedBuffer : public Buffer {
    
  public:

    virtual size_t sizeBytes()     {return this->size()*sizeof(T);} ///bytes in buffer
    virtual size_t capacityBytes() {return this->capacity()*sizeof(T);} ///bytes in buffer
    
    //basic element adding functions
    virtual size_t push(const T&) = 0; ///push one on
    virtual size_t pop(T&) = 0; ///pop one off

  protected:
    
  };

}

#endif  // app_framework_base_Buffers_TemplatedBuffer_hh
