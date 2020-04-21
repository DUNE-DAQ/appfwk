#ifndef app_framework_base_Buffers_Buffer_hh
#define app_framework_base_Buffers_Buffer_hh

#include "BufferI.hh"

#include <cstddef>
#include <utility>
using std::size_t;

namespace appframework{

  template <class T>
  class Buffer : public BufferI {
    
  public:

    virtual size_t capacityBytes() {return this->capacity()*sizeof(T);} ///bytes in buffer

  protected:

  };


  template <class T>
  class BufferInput : virtual public Buffer<T>{

  public:
    virtual int push(const T& t) { return push( std::move(t) ); }
    virtual int push(T&& ) = 0;

  protected:
    size_t  fPushTimeout_ms;
    
  };

  template <class T>
  class BufferOutput : virtual public Buffer<T>{

  public:
    virtual T pop() = 0;

  protected:
    size_t  fPopTimeout_ms ;
  };


}

#endif  // app_framework_base_Buffers_Buffer_hh
