#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_

#include "appfwk/cmd/Structs.hpp"
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

    // Execute a properly structured command
    void execute( const dataobj_t& cmd_data );

protected:
    typedef std::map<std::string, std::shared_ptr<DAQModule>> DAQModuleMap; ///< DAQModules indexed by name

    void initialize( const dataobj_t& data );
    void init_queues( const cmd::QueueSpecs& qspecs );
    void init_modules( const cmd::ModSpecs& mspecs );

    void dispatch(cmd::CmdId id, const dataobj_t& data );

    std::vector<std::shared_ptr<DAQModule>> match(std::string name);
private:

    bool initialized_;

    DAQModuleMap modulemap_;
};

}
}
#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
