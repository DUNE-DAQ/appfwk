/**
 * @file ConfigurationManagerOwner.hpp Instantiates and owns a ConfigurationManager
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_SRC_CONFIGURATIONMANAGEROWNER_HPP_
#define APPFWK_SRC_CONFIGURATIONMANAGEROWNER_HPP_

#include "appfwk/ConfigurationManager.hpp"

#include <memory>
#include <string>

namespace dunedaq::appfwk {

/**
 * This class exists to be a base class of Application, so that the ConfigurationManager can be initialized once and
 * used to initialize the OpMonManager (which is also a base class of Application).
 *
 * @todo ELF April 3, 2025: Determine how the ConfigurationManager should be exposed by this class to those using it
 */
class ConfigurationManagerOwner
{
public:
  ConfigurationManagerOwner(std::string confimpl, std::string app_name, std::string session_name)
    : m_config_mgr(std::make_shared<ConfigurationManager>(confimpl, app_name, session_name))
  {
  }

  std::shared_ptr<ConfigurationManager> get_config_manager() const { return m_config_mgr; }

private:
  std::shared_ptr<ConfigurationManager> m_config_mgr;
};

} // namespace dunedaq::appfwk

#endif // APPFWK_SRC_CONFIGURATIONMANAGEROWNER_HPP_
