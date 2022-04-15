/**
 * @file DAQModuleHelper.cpp DAQModule helper functions
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModuleHelper.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/app/Nljs.hpp"

#include <string>
#include <vector>

namespace dunedaq::appfwk {

IndexedConnectionRefs_t
connection_index(const nlohmann::json& iniobj, std::vector<std::string> required)
{
  IndexedConnectionRefs_t ret;
  for (auto cr : connection_refs(iniobj)) {
    ret[cr.name] = cr;
  }
  for (auto name : required) {
    if (ret.find(name) == ret.end()) {
      throw InvalidSchema(ERS_HERE, "missing queue: " + name);
    }
  }
  return ret;
}

iomanager::connection::ConnectionRefs_t
connection_refs(const nlohmann::json& iniobj)
{
  return iniobj.get<app::ModInit>().conn_refs;
}

iomanager::connection::ConnectionRef
connection_inst(const nlohmann::json& iniobj, const std::string& name)
{
  return connection_index(iniobj, { name })[name];
}

} // namespace dunedaq::appfwk
