#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_

#include <nlohmann/json.hpp>
#include <ers/Issue.h>

namespace dunedaq {

/**
 * @brief A generic DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,                 ///< Namespace
                  DAQModuleManagerNotInitialized,  ///< Issue class name
                  "Command " << cmdid << " received before initialization", ///< Message
                  ((std::string)cmdid)     ///< Message parameters
)

namespace appfwk {

class DAQModule;

class DAQModuleManager
{
public:

    using dataobj_t = nlohmann::json;

    DAQModuleManager();

    bool initialized() const { return initialized_; }

    void execute( const dataobj_t& cmd_data );

protected:
    typedef std::map<std::string, std::shared_ptr<DAQModule>> DAQModuleMap; ///< DAQModules indexed by name

    void initialize( const dataobj_t& init_data );

    void init_queues( const dataobj_t& queue_data );

private:

    bool initialized_;

    DAQModuleMap modulemap_;
};

}
}
#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
