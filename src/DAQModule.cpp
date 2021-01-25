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
  if (auto cmd = commands_.find(name); cmd != commands_.end()) {
    std::invoke(cmd->second, data);
    interrupt();
    return;
  }

  throw UnknownCommand(ERS_HERE, get_name(), name);
}

std::vector<std::string>
DAQModule::get_commands() const
{
  std::vector<std::string> cmds(commands_.size());
  for (const auto& [key, value] : commands_)
    cmds.push_back(key);
  return cmds;
}

bool
DAQModule::has_command(const std::string& name) const
{
  return (commands_.find(name) != commands_.end());
}

bool
DAQModule::interruptible_wait(std::chrono::microseconds wait_duration, std::function<bool()> wait_condition)
{
  std::unique_lock<std::mutex> wait_lock(m_wait_mutex_);
  return m_wait_cv_.wait_for(wait_lock, wait_duration, wait_condition);
}

} // namespace dunedaq::appfwk
