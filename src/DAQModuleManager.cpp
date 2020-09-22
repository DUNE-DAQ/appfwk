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
    // The command dispatch protocol
    auto cmdobj = data.get<cmd::CmdObj>();
    for (const auto& addressed : cmdobj.modules) {
        for ( auto mptr : match(addressed.match) ) {
            ERS_INFO("dispatch \""<<id<<"\" to \"" << mptr->get_name()
                     << "\":\n" << addressed.data.dump(4));
            mptr->execute_command(id, addressed.data);
        }
    }
}


std::vector<std::shared_ptr<DAQModule>>
DAQModuleManager::match(std::string name)
{
    if (name.empty()) { name = ".*"; }

    std::vector<std::shared_ptr<DAQModule>> ret;
    for (auto const& [nm, mptr] : modulemap_) {
        if (! std::regex_match(nm.c_str(), std::regex(name.c_str())) ) {
            continue;
        }
        ret.push_back(mptr);
    }
    return ret;
}


void
DAQModuleManager::execute( const dataobj_t& cmd_data ) {

    auto cmd = cmd_data.get<cmd::Command>();
    ERS_INFO("Command id:"<< cmd.id);

    if (cmd_data.contains("waitms")) {
        int waitms = cmd_data["waitms"];
        std::this_thread::sleep_for (std::chrono::milliseconds(waitms));
    }

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
