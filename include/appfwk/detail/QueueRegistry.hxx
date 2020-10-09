#include "appfwk/FollyQueue.hpp"
#include "appfwk/StdDeQueue.hpp"

#include <cxxabi.h>

// Declarations
namespace dunedaq::appfwk {

template<typename T>
std::shared_ptr<Queue<T>>
QueueRegistry::get_queue(const std::string& name)
{

  auto itQ = queue_registry_.find(name);
  if (itQ != queue_registry_.end()) {
    auto queuePtr = std::dynamic_pointer_cast<Queue<T>>(itQ->second.instance);

    if (!queuePtr) {
      // TODO: John Freeman (jcfree@fnal.gov), Jun-23-2020. Add checks for demangling status. Timescale 2 weeks.
      int status = -999;
      std::string realname_target = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
      std::string realname_source = abi::__cxa_demangle(itQ->second.type->name(), 0, 0, &status);

      throw QueueTypeMismatch(ERS_HERE, name, realname_source, realname_target);
    }

    return queuePtr;
  }

  auto itP = this->queue_configmap_.find(name);
  if (itP != queue_configmap_.end()) {
    QueueEntry entry = { &typeid(T), create_queue<T>(name, itP->second) };
    queue_registry_[name] = entry;
    return std::dynamic_pointer_cast<Queue<T>>(entry.instance);

  } else {
    // TODO: John Freeman (jcfree@fnal.gov), Jun-23-2020. Add checks for demangling status. Timescale 2 weeks.
    int status = -999;
    std::string realname_target = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    throw QueueNotFound(ERS_HERE, name, realname_target);
  }
}

template<typename T>
std::shared_ptr<Named>
QueueRegistry::create_queue(const std::string& name, const QueueConfig& config)
{

  std::shared_ptr<Named> queue;
  switch (config.kind) {
    case QueueConfig::kStdDeQueue:
      queue = std::make_shared<StdDeQueue<T>>(name, config.capacity);
      break;
    case QueueConfig::kFollySPSCQueue:
      queue = std::make_shared<FollySPSCQueue<T>>(name, config.capacity);
      break;
    case QueueConfig::kFollyMPMCQueue:
      queue = std::make_shared<FollyMPMCQueue<T>>(name, config.capacity);
      break;

    default:
      throw QueueKindUnknown(ERS_HERE, std::to_string(config.kind));
  }

  return queue;
}

} // namespace dunedaq::appfwk
