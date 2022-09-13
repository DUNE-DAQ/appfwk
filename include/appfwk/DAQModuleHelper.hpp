/**
 * @file DAQModuleHelper.hpp Some utility functions for writing modules.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULEHELPER_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULEHELPER_HPP_

#include "appfwk/app/Structs.hpp"

#include <map>
#include <string>
#include <vector>

namespace dunedaq {

namespace appfwk {

using NameUidMap_t = std::map<std::string, std::string>;

/// @brief Return map of QueueInfo indexed by their "name".
///
/// The iniobj is as given to a module's init() method.
NameUidMap_t
connection_index(const nlohmann::json& iniobj, std::vector<std::string> required = {});

/// @brief Return vector of QueueInfo from the init() object
app::ConnectionReferences_t
connection_refs(const nlohmann::json& iniobj);

std::string
connection_uid(const nlohmann::json& iniobj, const std::string& name);

} // namespace appfwk

} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEHELPER_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
