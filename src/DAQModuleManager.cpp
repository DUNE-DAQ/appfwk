#include "appfwk/DAQModuleManager.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "appfwk/DAQModule.hpp"
#include "appfwk/QueueRegistry.hpp"

#include "ers/ers.h"

#include <regex>
#include <vector>
#include <utility>

namespace dunedaq {
namespace appfwk {

DAQModuleManager::DAQModuleManager() :
    initialized_(false) {

}


void
DAQModuleManager::initialize( const dataobj_t& data) {
    auto ini = data.get<cmd::Init>();
    init_queues(ini.queues);
    init_modules(ini.modules);
    this->initialized_ = true;
}

void
DAQModuleManager::init_modules(const cmd::ModSpecs& mspecs) {
    for (const auto& mspec : mspecs) {
        ERS_INFO("construct: " << mspec.plugin << " : " << mspec.inst);
        auto mptr = makeModule(mspec.plugin, mspec.inst);
        modulemap_.emplace(mspec.inst, mptr);
        mptr->init(mspec.data);
    }
}


void
DAQModuleManager::init_queues(const cmd::QueueSpecs& qspecs) {
    std::map<std::string, QueueConfig> qrcfg;
    for (const auto& qs : qspecs) {

        // N.B.: here we mimic the behavior of daq_application and
        // ignore the kind.  This requires user configuration to
        // assure unique queue names across all queue types.
        const std::string qname = qs.inst;
        // fixme: maybe one day replace QueueConfig with codgen.
        // Until then, wheeee....
        QueueConfig qc;
        switch (qs.kind) {
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
DAQModuleManager::dispatch_after_merge(cmd::CmdId id, const dataobj_t& data) {
    // The command dispatching: commands and parameters are distributed to all modules that
    // have registered a method corresponding to the command. If no parameters are found, an
    // empty dataobj_t is passed.
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
            ERS_LOG("Dispatch \"" << id << "\" to \"" << mod_ptr->get_name()
                    << "\":\n" << params.dump(4));
            try {
                mod_ptr->execute_command(id, params);
            } catch (ers::Issue& ex) {
                ers::error(ex);
                bad_mod_names.append(mod_name);
                bad_mod_names.append(", ");
            }
        }
    }
    if (!bad_mod_names.empty()) {
        throw CommandDispatchingFailed(ERS_HERE, id, bad_mod_names);
    }
}


std::vector<std::string>
DAQModuleManager::get_modnames_by_cmdid(cmd::CmdId id) {
    // Make a convenience array with module names that have the requested command
    std::vector<std::string> mod_names;
    for (const auto& [mod_name, mod_ptr] : modulemap_) {
        if (mod_ptr->has_command(id))
            mod_names.push_back(mod_name);
    }

    return mod_names;
}


void
DAQModuleManager::dispatch_one_match_only(cmd::CmdId id, const dataobj_t& data) {
    // This method ensures that each module is only matched once per command.
    // If multiple matches are found, an ers::Issue is thrown
    // Disclaimenr for the occasional reader: this is the first implementation of the 
    // multiple-matches detection logic. The author is painfully aware that it can be 
    // vastly improved, in style if not in performance.

    auto cmd_obj = data.get<cmd::CmdObj>();
    const dataobj_t dummy{};

    // Make a convenience array with module names that have the requested command
    std::vector<std::string> cmd_mod_names = get_modnames_by_cmdid(id);

    // containers for error tracking
    std::vector<std::string> unmatched_addr;
    std::map<std::string, std::vector<std::string> > mod_to_re;

    std::vector<std::pair<std::vector<std::string>, const dataobj_t* >> mod_seq;

    if (!cmd_obj.modules.empty()) {
        for (const auto& addressed : cmd_obj.modules) {

            // Module names matching the 'match' regex
            std::vector<std::string> matches;

            // First exception: empty = `all`
            if (addressed.match.empty()) {
                matches = cmd_mod_names;
            } else {
                // Find module names matching the regex
                for ( const std::string& mod_name : cmd_mod_names ) {
                    // match module name with regex
                    if ( std::regex_match(mod_name, std::regex(addressed.match)) ) {
                        matches.push_back(mod_name);
                        mod_to_re[mod_name].push_back(addressed.match);
                    }
                }

                // Keep track of unmatched expressions
                if (matches.empty()) {
                    unmatched_addr.push_back(addressed.match);
                    continue;
                }
            }
            mod_seq.emplace_back( matches, &addressed.data);
        }

        if ( !unmatched_addr.empty() ) {
            // say something!
            // or not?
        }

        // Select modules with multiple matches
        for (auto i = mod_to_re.begin(), last = mod_to_re.end(); i != last; ) {
            if (i->second.size() == 1) {
                i = mod_to_re.erase(i);
            } else {
                ++i;
            }
        }

        // Catch cases 
        if (mod_to_re.size() > 0) {
            std::string mod_names;
            for( const auto& [mod_name, matched_re] : mod_to_re ) {
                mod_names += mod_name + ", ";
            }
            throw ConflictingCommandMatching(ERS_HERE, id, mod_names);
        }

    } else {
        mod_seq.emplace_back(cmd_mod_names, &dummy);
    }
    
    std::string failed_mod_names("");

    // All sorted, execute!
    for ( auto& [mod_names, data_ptr] : mod_seq) {
        for ( auto& mod_name : mod_names ) {
            try {
                ERS_LOG( "Executing " << id << " -> " << mod_name );
                modulemap_[mod_name]->execute_command(id, *data_ptr);
            } catch (ers::Issue& ex) {
                ers::error(ex);
                failed_mod_names.append(mod_name);
                failed_mod_names.append(", ");
            }
        }
    }

    // Throw if any dispatching failed
    if (!failed_mod_names.empty()) {
        throw CommandDispatchingFailed(ERS_HERE, id, failed_mod_names);
    }
}


void
DAQModuleManager::execute( const dataobj_t& cmd_data ) {

    auto cmd = cmd_data.get<cmd::Command>();
    ERS_LOG("Command id:" << cmd.id);

    if ( ! initialized_ ) {
        if ( cmd.id != "init" ) {
            throw DAQModuleManagerNotInitialized(ERS_HERE, cmd.id);
        }
        this->initialize( cmd.data );
        return;
    }


    dispatch_one_match_only(cmd.id, cmd.data);

    // dispatch(cmd.id, cmd.data);
}


} // namespace appfwk
} // namespace dunedaq
