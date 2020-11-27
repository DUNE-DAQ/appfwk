#include "appfwk/DAQModuleManager.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "appfwk/DAQModule.hpp"
#include "appfwk/QueueRegistry.hpp"

#include "ers/ers.h"

#include <regex>

namespace dunedaq {
namespace appfwk {

DAQModuleManager::DAQModuleManager() :
    initialized_(false)
{

}


void
DAQModuleManager::initialize( const dataobj_t& data)
{
    auto ini = data.get<cmd::Init>();
    init_queues(ini.queues);
    init_modules(ini.modules);
    this->initialized_ = true;
}

void 
DAQModuleManager::init_modules(const cmd::ModSpecs& mspecs)
{
    for (const auto& mspec : mspecs) {
        ERS_INFO("construct: " << mspec.plugin << " : " << mspec.inst);
        auto mptr = makeModule(mspec.plugin, mspec.inst);
        modulemap_.emplace(mspec.inst, mptr);
        mptr->init(mspec.data);
    }
}


void 
DAQModuleManager::init_queues(const cmd::QueueSpecs& qspecs)
{
    std::map<std::string, QueueConfig> qrcfg;
    for (const auto& qs : qspecs) {

        // N.B.: here we mimic the behavior of daq_application and
        // ignore the kind.  This requires user configuration to
        // assure unique queue names across all queue types.
        const std::string qname = qs.inst;
        // fixme: maybe one day replace QueueConfig with codgen.
        // Until then, wheeee....
        QueueConfig qc;
        switch(qs.kind) {
        case cmd::QueueKind::StdDeQueue:
            qc.kind = QueueConfig::queue_kind::kStdDeQueue;
            break;
        case cmd::QueueKind::FollySPSCQueue:
            qc.kind = QueueConfig::queue_kind::kFollySPSCQueue;
            break;
        case cmd::QueueKind::FollyMPMCQueue:
            qc.kind = QueueConfig::queue_kind::kFollyMPMCQueue;
            break;
        default:
            throw MissingComponent(ERS_HERE, "unknown queue type");
            break;
        }
        qc.capacity = qs.capacity;
        qrcfg[qname] = qc;
        ERS_INFO("Adding queue: " << qname);
    }
    QueueRegistry::get().configure(qrcfg);
}

void
DAQModuleManager::dispatch(cmd::CmdId id, const dataobj_t& data)
{
    // The command dispatching: commands and parameters are distributed to all modules that
    // have registered a method corresponding to the command. If no parameters are found, an
    // empty dataobj_t is passed.
    bool fault = false;
    std::string bad_mod_names("");
    auto cmd_obj = data.get<cmd::CmdObj>();
    for (const auto& [mod_name, mod_ptr] : modulemap_) {
	if (mod_ptr->has_command(id)) {
	    dataobj_t params;
	    for (const auto& addressed : cmd_obj.modules) {
                if (addressed.match.empty() || std::regex_match(mod_name.c_str(), std::regex(addressed.match.c_str()))  ) {
		    for (nlohmann::json::const_iterator it = addressed.data.begin(); it != addressed.data.end(); ++it) {
        	        params[it.key()] = it.value();
    		    }
		}
	    }
            ERS_LOG("Dispatch \""<<id<<"\" to \"" << mod_ptr->get_name()
                     << "\":\n" << params.dump(4));
            try {
		mod_ptr->execute_command(id, params);
	    }
	    catch(ers::Issue& ex) {
		ers::error(ex);
		fault=true;
		bad_mod_names.append(mod_name);
		bad_mod_names.append(", ");	 
	    }
	}
    }
    if(fault) {
	throw CommandDispatchingFailed(ERS_HERE, id, bad_mod_names);
    }
}

void
DAQModuleManager::execute( const dataobj_t& cmd_data ) {

    auto cmd = cmd_data.get<cmd::Command>();
    ERS_LOG("Command id:"<< cmd.id);

    if ( ! initialized_ ) {
        if ( cmd.id != "init" ) {
            throw DAQModuleManagerNotInitialized(ERS_HERE, cmd.id);
        }
        this->initialize( cmd.data );
        return;
    }

    dispatch(cmd.id, cmd.data);
}


} // namespace appfwk
} // namespace dunedaq
