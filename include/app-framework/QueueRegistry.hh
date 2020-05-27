
#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HH_

#include <map>
#include <memory>
#include <string>
#include <app-framework-base/NamedObject.hh>
#include <app-framework/Queues/NamedStdDeQueue.hh>

namespace appframework {

struct QueueConfig
{
  enum queue_kind { std_deque };

  QueueConfig::queue_kind kind;
  size_t size;
};

class QueueRegistry {
  public:


    ~QueueRegistry();

    static QueueRegistry* get();

    template<typename T>
    std::shared_ptr<NamedQueueI<T>> get_queue(std::string name);

    void configure( const std::map<std::string, QueueConfig>& );

  private:
    QueueRegistry();

    template<typename T>
    std::shared_ptr<NamedObject> create_queue(std::string name, const QueueConfig& config);

    std::map<std::string,std::shared_ptr<NamedObject> > queue_registry_;
    std::map<std::string,QueueConfig> queue_configmap_;
    bool configured_;

    static QueueRegistry* me_;

    QueueRegistry(const QueueRegistry&) = delete;
    QueueRegistry& operator=(const QueueRegistry&) = delete;
    QueueRegistry(QueueRegistry&&) = delete;
    QueueRegistry& operator=(QueueRegistry&&) = delete;

};


template<typename T>
std::shared_ptr<NamedQueueI<T>> QueueRegistry::get_queue(std::string name) {

  auto itQ = queue_registry_.find(name);
  if ( itQ != queue_registry_.end() ) {
    return std::dynamic_pointer_cast<NamedQueueI<T>>(itQ->second);
  } 

  auto itP = queue_configmap_.find(name);
  if ( itP != queue_configmap_.end() ) {
    std::shared_ptr<NamedObject> queue = create_queue<T>(name, itP->second);
    queue_registry_[name] = queue;
    return std::dynamic_pointer_cast<NamedQueueI<T>>(queue);

  } else {
    throw std::runtime_error("Queue not found");
  }

}


template<typename T>
std::shared_ptr<NamedObject> QueueRegistry::create_queue(std::string name, const QueueConfig& config) {

  std::shared_ptr<NamedObject> queue;
  switch(config.kind) {
    case QueueConfig::std_deque:
      queue = std::make_shared<NamedStdDeQueue<T>>(name, config.size);
      break;
    default:
      throw std::runtime_error("Unknown queue kind");
  }

  return queue;
}


} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HH_