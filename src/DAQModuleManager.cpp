#include "appfwk/DAQModuleManager.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/app/Nljs.hpp"

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
DAQModuleManager::initialize( const dataobj_t& init_data) {

    auto ads = init_data.get<app::Addressed>();
    
    // Find the queue object
    // How to guarantee uniqueness?
    for (const auto& ad : ads.addrdats) {
        if ( ad.ki.kind != "queue" ) continue;

        init_queues(ad.data);
    }

    // loop over modules
    for (const auto& ad : ads.addrdats) {

        if ( ad.ki.kind == "module" ) {
            auto mi = ad.data.get<app::ModInit>();

            ERS_INFO("construct: " << mi.plugin << " : " << ad.ki.inst);
            auto mptr = makeModule(mi.plugin, ad.ki.inst);
            modulemap_.emplace(ad.ki.inst, mptr);

            mptr->init(mi.data);
        }
    }

    this->initialized_ = true;
}


void 
DAQModuleManager::init_queues(const dataobj_t& queue_data) {
    auto ads = queue_data.get<app::Addressed>();

    std::map<std::string, QueueConfig> qrcfg;
    for (auto& ad : ads.addrdats) {
        auto qi = ad.data.get<app::QueueInit>();
        // N.B.: here we mimic the behavior of daq_application and
        // ignore the kind.  This requires user configuration to
        // assure unique queue names across all queue types.
        const std::string qname = ad.ki.inst;
        // fixme: maybe one day replace QueueConfig with codgen.
        // Until then, wheeee....
        QueueConfig qc;
        switch(qi.kind) {
        case app::QueueKind::StdDeQueue:
            qc.kind = QueueConfig::queue_kind::kStdDeQueue;
            break;
        case app::QueueKind::FollySPSCQueue:
            qc.kind = QueueConfig::queue_kind::kFollySPSCQueue;
            break;
        case app::QueueKind::FollyMPMCQueue:
            qc.kind = QueueConfig::queue_kind::kFollyMPMCQueue;
            break;
        default:
            throw MissingComponent(ERS_HERE, "unknown queue type");
            break;
        }
        qc.capacity = qi.capacity;
        qrcfg[qname] = qc;
        ERS_INFO("Adding queue: " << qname);
    }
    QueueRegistry::get().configure(qrcfg);
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

    auto cmdobj = cmd_data.get<cmd::Command>();
    ERS_INFO("Command id:"<< cmdobj.id);

    if ( ! initialized_ ) {

        if ( cmdobj.id != "init" ) {
            throw DAQModuleManagerNotInitialized(ERS_HERE, cmdobj.id);
        }
        ERS_INFO("Dispatch init construction with:\n" << cmdobj.data.dump(4));
        this->initialize( cmdobj.data );
    } else {


        auto ads = cmdobj.data.get<app::Addressed>();
        for (const auto& ad : ads.addrdats) {
            ERS_INFO("---" << ad.ki.kind << "  " << ad.ki.inst );

            if ( ad.ki.kind != "module" )
                continue;

            for ( auto mptr : match(ad.ki.inst) ) {
                mptr->execute_command(cmdobj.id, ad.data);
            }
        }
    }

}


} // namespace appfwk
} // namespace dunedaq
