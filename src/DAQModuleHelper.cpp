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

IndexedQueueInfos_t
queue_index(const nlohmann::json& iniobj, std::vector<std::string> required)
{
  IndexedQueueInfos_t ret;
  for (auto qi : queue_infos(iniobj)) {
    ret[qi.name] = qi;
  }
  for (auto name : required) {
    if (ret.find(name) == ret.end()) {
      throw InvalidSchema(ERS_HERE, "missing queue: " + name);
    }
  }
  return ret;
}

app::QueueInfos
queue_infos(const nlohmann::json& iniobj)
{
  return iniobj.get<app::ModInit>().qinfos;
}
} // namespace dunedaq::appfwk
