/**
 * @file QueueRegistry.cc 
 *
 * The QueueRegistry class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/QueueRegistry.hh"

namespace appframework {

QueueRegistry* QueueRegistry::me_ = nullptr;

QueueRegistry::QueueRegistry()
  : configured_(false)
{}

QueueRegistry::~QueueRegistry() {}

QueueRegistry & 
QueueRegistry::get()
{
  if (!me_) {
    me_ = new QueueRegistry();
  }
  return *me_;
}

void
QueueRegistry::configure(const std::map<std::string, QueueConfig>& configmap)
{
  if (configured_) {
    throw std::runtime_error("QueueRegistry already configured");
  }

  queue_configmap_ = configmap;
  configured_ = true;
}

} // namespace appframework
