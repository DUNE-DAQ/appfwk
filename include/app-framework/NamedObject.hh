/**
 * @file NamedObject.hh NamedObject class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_NAMEDOBJECT_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_NAMEDOBJECT_HH_

#include <string>

namespace appframework {
class NamedObject
{
public:
  /**
   * @brief NamedObject Constructor
   * @param name Name of this object
  */
  explicit NamedObject(const std::string& name)
    : name_(name)
  {}
  NamedObject(NamedObject const&) = delete;
  NamedObject(NamedObject&&) = default;
  NamedObject& operator=(NamedObject const&) = delete;
  NamedObject& operator=(NamedObject&&) = default;
  virtual ~NamedObject() = default;

  /**
   * @brief Get the name of this NamedObejct
   * @return The name of this NamedObject
  */
  const std::string& get_name() const { return name_; }

private:
  std::string name_;
};
} // namespace appframework
#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_NAMEDOBJECT_HH_
