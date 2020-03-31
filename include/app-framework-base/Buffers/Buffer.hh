/**
 * @file Buffer class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Buffers_Buffer_hh
#define app_framework_base_Buffers_Buffer_hh

namespace appframework {
/**
 * @brief Implementations of the Buffer class are responsible for relaying data between UserModules within a DAQ Application
 *
 * Note that while the Buffer class itself is not templated on a data type (so it can be included in generic containers), all implementations should be.
 */
class Buffer {};

}  // namespace appframework

#endif  // app_framework_base_Buffers_Buffer_hh