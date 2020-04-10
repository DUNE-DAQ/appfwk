/**
 * @file Buffer class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Buffers_Buffer_hh
#define app_framework_base_Buffers_Buffer_hh

#include <cstddef>
using std::size_t;

namespace appframework {
  /**
   * @brief Implementations of the Buffer class are responsible for relaying data between UserModules within a DAQ Application
   *
   * Note that while the Buffer class itself is not templated on a data type (so it can be included in generic containers), all implementations should be.
   */
  class Buffer {
    
  public:

    virtual void Configure() = 0; ///called when configured in FSM
    
    bool isBounded() { return fBounded; }

    virtual size_t size() = 0; ///elements in buffer
    //virtual size_t sizeBytes() = 0; ///bytes in buffer
    
    virtual size_t capacity() = 0;
    //virtual size_t capacityBytes() =0; ///capacity in bytes of buffer

    //basic element adding functions
    //virtual size_t push(const T&) = 0; ///push one on
    //virtual size_t pop(T&) = 0; ///pop one off


  protected:
    
    bool fBounded;  ///can size increase dynamically?

  };

}

#endif  // app_framework_base_Buffers_Buffer_hh
