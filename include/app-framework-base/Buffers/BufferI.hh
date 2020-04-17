/**
 * @file Buffer class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Buffers_BufferI_hh
#define app_framework_base_Buffers_BufferI_hh

#include <cstddef>
using std::size_t;

namespace appframework {
  /**
   * @brief Attributes of Buffers that can be communicated to framework for DAQProcess configuration and testing.
   * 
   */
  struct BufferAttributes{
    bool isBounded;     ///<can size increase dynamically?
    bool isSearchable;  ///<is the data in the buffer searchable?
  };



  /**
   * @brief Implementations of the Buffer class are responsible for relaying data between UserModules within a DAQ Application
   *
   * Note that while the Buffer class itself is not templated on a data type (so it can be included in generic containers), all implementations should be.
   */
  class BufferI {
    
  public:

    virtual void Configure() = 0; ///<called when configured in FSM
    
    BufferAttributes attributes() { return fAttributes; }

    virtual bool   empty() = 0;    ///<is there any available data?
    virtual size_t capacity() = 0; ///<what is the available capacity?

  protected:
    
    BufferAttributes fAttributes;  ///buffer attributes

  };

} //namespace appframework

#endif  // app_framework_base_Buffers_Buffer_hh
