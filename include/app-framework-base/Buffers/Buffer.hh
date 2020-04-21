#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_

/**
 * @file Buffer class interface which augments BufferI with push and pop functions
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */


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


} // namespace appframework  

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_BUFFERS_BUFFER_HH_
