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

using IndexedConnectionRefs_t = std::map<std::string, iomanager::connection::ConnectionRef>;

/// @brief Return map of QueueInfo indexed by their "name".
///
/// The iniobj is as given to a module's init() method.
IndexedConnectionRefs_t
connection_index(const nlohmann::json& iniobj, std::vector<std::string> required = {});

/// @brief Return vector of QueueInfo from the init() object
iomanager::connection::ConnectionRefs_t
connection_refs(const nlohmann::json& iniobj);

iomanager::connection::ConnectionRef
connection_inst(const nlohmann::json& iniobj, const std::string& name);

struct queue_info
{
  std::string inst;
};
using IndexedQueueInfos_t = std::map<std::string, queue_info>;

[[deprecated("Use connection_index")]] IndexedQueueInfos_t
queue_index(const nlohmann::json& iniobj, std::vector<std::string> required = {})
{
  IndexedQueueInfos_t ret;
  auto idx = connection_index(iniobj, required);
  for (auto& i : idx) {
    ret[i.first].inst = i.second.uid;
  }
  return ret;
}

[[deprecated("Use connection_inst")]] std::string
queue_inst(const nlohmann::json& iniobj, const std::string& name)
{
  return connection_inst(iniobj, name).uid;
}

} // namespace appfwk

} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEHELPER_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
