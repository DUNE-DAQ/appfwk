/**
 * @file Configurable.hpp Instantiates and owns a ConfigurationManager
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_CONFIGURABLE_HPP_
#define APPFWK_INCLUDE_APPFWK_CONFIGURABLE_HPP_

#include "appfwk/ConfigurationManager.hpp"

#include <memory>
#include <string>

namespace dunedaq {
namespace appfwk {

class Configurable
{
public:
  Configurable(std::string confimpl, std::string app_name, std::string session_name)
    : m_config_mgr(std::make_shared<ConfigurationManager>(confimpl, app_name, session_name))
  {
  }

  std::shared_ptr<ConfigurationManager> get_config_manager() const { return m_config_mgr; }

private:
  std::shared_ptr<ConfigurationManager> m_config_mgr;
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_CONFIGURABLE_HPP_
