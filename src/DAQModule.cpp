/**
 * @file DAQModule.cpp DAQModule class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModule.hpp"

#include <string>
#include <vector>

namespace dunedaq::appfwk {

void
DAQModule::execute_command(const std::string& name, const data_t& data)
{
  if (auto cmd = m_commands.find(name); cmd != m_commands.end()) {
    std::invoke(cmd->second, data);
    interrupt();
    return;
  }

  throw UnknownCommand(ERS_HERE, get_name(), name);
}

std::vector<std::string>
DAQModule::get_commands() const
{
  std::vector<std::string> cmds(m_commands.size());
  for (const auto& [key, value] : m_commands)
    cmds.push_back(key);
  return cmds;
}

bool
DAQModule::has_command(const std::string& name) const
{
  return (m_commands.find(name) != m_commands.end());
}

} // namespace dunedaq::appfwk
