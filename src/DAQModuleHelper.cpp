/**
 * @file DAQModuleHelper.cpp DAQModule helper functions
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModuleHelper.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include <string>
#include <vector>

namespace dunedaq::appfwk {

IndexedQueueInfos qindex(const nlohmann::json& iniobj,
                             std::vector<std::string> required)
{
    IndexedQueueInfos ret;
    for (auto qi : qinfos(iniobj)) {
        ret[qi.name] = qi;
    }
    for (auto name : required) {
        if (ret.find(name) == ret.end()) {
            throw SchemaError(ERS_HERE, "missing queue: " + name);
        }
    }
    return ret;
}
    
cmd::QueueInfos qinfos(const nlohmann::json& iniobj)
{
    return iniobj.get<cmd::ModInit>().qinfos;
}
} // namespace dunedaq::appfwk
