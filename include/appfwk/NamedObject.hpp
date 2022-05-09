/**
 * @file NamedObject.hpp NamedObject Adapter
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_NAMEDOBJECT_HPP_
#define APPFWK_INCLUDE_APPFWK_NAMEDOBJECT_HPP_

#pragma message("This header is deprecated, use #include \"utilities/NamedObject.hpp\" instead!")

#include "utilities/NamedObject.hpp"

namespace dunedaq {
namespace appfwk {
using utilities::Named;
using utilities::NamedObject;
} // namespace appfwk

} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_NAMEDOBJECT_HPP_