/**
 * @file DAQModule.hxx DAQModule Inline Method Definitions
 *
 * N.B. register_command is defined here as it is a template method, and must be visible so that the compiler can
 * generate type-specific implementations as needed. make_module is a free function in the appfwk namespace, defined
 * here to reduce the clutter in DAQModule.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

namespace dunedaq::appfwk {

template<typename Child>
void
DAQModule::register_command(const std::string& cmd_name, void (Child::*f)(const data_t&))
{
  using namespace std::placeholders;

  bool done =
    m_commands.emplace(cmd_name, std::bind(f, dynamic_cast<Child*>(this), _1)).second; // NOLINT(modernize-avoid-bind)
  if (!done) {
    // Throw here
    throw CommandRegistrationFailed(ERS_HERE, get_name(), cmd_name);
  }
}

/**
 * @brief Load a DAQModule plugin and return a shared_ptr to the contained DAQModule class
 * @param plugin_name Name of the plugin, e.g. DebugLoggingDAQModule
 * @param instance_name Name of the returned DAQModule instance, e.g. DebugLogger1
 * @return shared_ptr to created DAQModule instance
 */
inline std::shared_ptr<DAQModule>
make_module(std::string const& plugin_name, std::string const& instance_name)
{
  static cet::BasicPluginFactory bpf("duneDAQModule", "make");

  std::shared_ptr<DAQModule> mod_ptr;
  try {
    mod_ptr = bpf.makePlugin<std::shared_ptr<DAQModule>>(plugin_name, instance_name);
  } catch (const cet::exception& cexpt) {
    throw DAQModuleCreationFailed(ERS_HERE, plugin_name, instance_name, cexpt);
  }
  return mod_ptr;
}
} // namespace dunedaq::appfwk
