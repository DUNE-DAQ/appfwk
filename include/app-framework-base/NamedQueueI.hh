#ifndef APP_FRAMEWORK_BASE_NAMEDQUEUE_HH_
#define APP_FRAMEWORK_BASE_NAMEDQUEUE_HH_


#include <chrono>
#include <string>
#include "app-framework-base/NamedObject.hh"

namespace appframework {

template<class T>

class NamedQueueI : public NamedObject {
public:
  using value_type = T;
  using duration_type = std::chrono::milliseconds;

  NamedQueueI(std::string name) : NamedObject(name) {}

  virtual void push(T&& val, const duration_type& timeout) = 0;

  void push(const T& val, const duration_type& timeout)
  {
    push(T(val), timeout);
  }

  virtual bool can_push() const noexcept = 0;

  virtual T pop(const duration_type& timeout) = 0;

  virtual bool can_pop() const noexcept = 0;

private:
  NamedQueueI(const NamedQueueI&) = delete;
  NamedQueueI& operator=(const NamedQueueI&) = delete;
  NamedQueueI(NamedQueueI&&) = default;
  NamedQueueI& operator=(NamedQueueI&&) = default;
};

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_NAMEDQUEUE_HH_
