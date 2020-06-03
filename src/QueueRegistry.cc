#include "app-framework/QueueRegistry.hh"

namespace appframework {

QueueRegistry* QueueRegistry::me_ = nullptr;

QueueRegistry::QueueRegistry()
  : configured_(false)
{}

QueueRegistry::~QueueRegistry() {}

QueueRegistry*
QueueRegistry::get()
{
  if (!me_) {
    me_ = new QueueRegistry();
  }
  return me_;
}

void
QueueRegistry::configure(const std::map<std::string, QueueConfig>& configmap)
{
  if (configured_) {
    throw std::runtime_error("QueueRegistry already configures");
  }

  queue_configmap_ = configmap;
  configured_ = true;
}

}