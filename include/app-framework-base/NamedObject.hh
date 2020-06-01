#ifndef APP_FRAMEWORK_BASE_NAMEDOBJECT_HH_
#define APP_FRAMEWORK_BASE_NAMEDOBJECT_HH_

#include <string>

#include "cetlib/BasicPluginFactory.h"
#include "cetlib/compiler_macros.h"
#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                            \
  extern "C"                                                                   \
  {
#endif

#define DEFINE_DUNE_OBJECT(klass)                                              \
  EXTERN_C_FUNC_DECLARE_START                                                  \
  std::shared_ptr<appframework::NamedObject> make(std::string n)               \
  {                                                                            \
    return std::shared_ptr<appframework::NamedObject>(new klass(n));           \
  }                                                                            \
  }

namespace appframework {
class NamedObject
{
public:
  NamedObject(const std::string& name)
    : name_(name)
  {}
  virtual ~NamedObject() = default;

  const std::string& get_name() const { return name_; }

private:
  std::string name_;
};

inline std::shared_ptr<NamedObject>
makeNamedObject(std::string const& plugin_name,
                std::string const& instance_name)
{
  static cet::BasicPluginFactory bpf("duneObject", "make");

  return bpf.makePlugin<std::shared_ptr<NamedObject>>(plugin_name,
                                                      instance_name);
}
} // namespace appframework
#endif // APP_FRAMEWORK_BASE_NAMEDOBJECT_HH_
