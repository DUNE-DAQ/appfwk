/**
 * @file DAQModuleHelper.hpp Some utility functions for writing modules.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULEHELPER_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULEHELPER_HPP_

#include "appfwk/cmd/Structs.hpp"

#include <map>
#include <vector>
#include <string>

namespace dunedaq {

namespace appfwk {

    using IndexedQueueInfos = std::map<std::string, cmd::QueueInfo>;

    /// @brief Return map of QueueInfo indexed by their "name".
    ///
    /// The iniobj is as given to a module's init() method.
    IndexedQueueInfos qindex(const nlohmann::json& iniobj,
                             std::vector<std::string> required = {});
    
    /// @brief Return vector of QueueInfo from the init() object
    cmd::QueueInfos qinfos(const nlohmann::json& iniobj);

} // namespace appfwk

} // namespace dunedaq


#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEHELPER_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
