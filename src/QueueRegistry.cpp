/**
 * @file QueueRegistry.cpp
 *
 * The QueueRegistry class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/QueueRegistry.hpp"

#include <map>
#include <memory>
#include <string>

namespace dunedaq::appfwk {

std::unique_ptr<QueueRegistry> QueueRegistry::s_instance = nullptr;

QueueRegistry&
QueueRegistry::get()
{
  if (!s_instance) {
    s_instance.reset(new QueueRegistry());
  }
  return *s_instance;
}

void
QueueRegistry::configure(const app::QueueSpecs& qspecs)
{
  if (m_configured) {
    throw QueueRegistryConfigured(ERS_HERE);
  }

  for (auto& qspec : qspecs) {
    m_queue_config_map[qspec.inst] = qspec;
  }
  m_configured = true;
}

void
QueueRegistry::gather_stats(opmonlib::InfoCollector& ic, int level)
{

  for (const auto& [name, queue_entry] : m_queue_registry) {
    opmonlib::InfoCollector tmp_ci;
    queue_entry.m_instance->get_info(tmp_ci, level);
    if (!tmp_ci.is_empty()) {
      ic.add(name, tmp_ci);
    }
  }
}

} // namespace dunedaq::appfwk
