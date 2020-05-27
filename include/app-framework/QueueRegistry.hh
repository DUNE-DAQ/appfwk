
#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HH_

#include <map>
#include <memory>
#include <string>
#include <app-framework-base/NamedObject.hh>
#include <app-framework/Queues/NamedStdDeQueue.hh>

namespace appframework {

class QueueRegistry {
  public:

    enum queue_kind { std_deque };

    ~QueueRegistry();

    static QueueRegistry* get();

    template<typename T>
    std::shared_ptr<NamedQueueI<T>> get_queue(std::string name, QueueRegistry::queue_kind kind, size_t size=0);

  private:
    QueueRegistry();

    template<typename T>
    std::shared_ptr<NamedObject> create_queue(std::string name, QueueRegistry::queue_kind kind, size_t size);

    std::map<std::string,std::shared_ptr<NamedObject> > registry_;

    static QueueRegistry* me_;

    QueueRegistry(const QueueRegistry&) = delete;
    QueueRegistry& operator=(const QueueRegistry&) = delete;
    QueueRegistry(QueueRegistry&&) = delete;
    QueueRegistry& operator=(QueueRegistry&&) = delete;

};


template<typename T>
std::shared_ptr<NamedQueueI<T>> QueueRegistry::get_queue(std::string name, QueueRegistry::queue_kind kind, size_t size) {

  auto it = registry_.find(name);
  if ( it != registry_.end() ) {
    return std::dynamic_pointer_cast<NamedQueueI<T>>(it->second);
  } else {
    std::shared_ptr<NamedObject> queue = create_queue<T>(name, kind, size);
    registry_[name] = queue;
    return std::dynamic_pointer_cast<NamedQueueI<T>>(queue);

  }

}


template<typename T>
std::shared_ptr<NamedObject> QueueRegistry::create_queue(std::string name, QueueRegistry::queue_kind kind, size_t size) {

  std::shared_ptr<NamedObject> queue;
  switch(kind) {
    case std_deque:
      queue = std::make_shared<NamedStdDeQueue<T>>(name, size);
      break;
    default:
      throw std::runtime_error("Unknown queue kind");
  }

  return queue;
}


} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HH_