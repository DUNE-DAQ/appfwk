#include "appfwk/DAQModuleHelper.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/cmd/Nljs.hpp"

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
}
