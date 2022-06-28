/**
 * @file ConfigurationWrapper.hpp ConfigurationWrapper helper class
 *
 * ConfigurationWrapper takes the configuration URI and produces
 * configuration objects for init and conf
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_CONFIGURATIONWRAPPER_HPP_
#define APPFWK_INCLUDE_APPFWK_CONFIGURATIONWRAPPER_HPP_

#include <logging/Logging.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

#include "boost/program_options.hpp"
#include "ers/ers.hpp"

#include <string>
#include <vector>

namespace dunedaq {
    ERS_DECLARE_ISSUE(appfwk, BadFile,
                     "Can not open file: " << filename,
                      ((std::string)filename))

    ERS_DECLARE_ISSUE(appfwk, CannotParseData,
                     "Can not parse data: " << reason,
                      ((std::string)reason))


} // namespace dunedaq

namespace dunedaq {

namespace appfwk {
/**
 * @brief ConfigurationWrapper parses the config uri given to the
 * application and form json data opbiects for init and conf
 */
struct ConfigurationWrapper
{
public:
	ConfigurationWrapper(const std::string& conf_uri, const std::string& app_name);
	nlohmann::json get_data(const std::string& cmd);
private:
	std::string m_conf_uri;
	std::string m_app_name;

};
} // namespace appfwk
} // namespace dunedaq

#include "detail/ConfigurationWrapper.hxx"

#endif // APPFWK_INCLUDE_APPFWK_CONFIGURATIONWRAPPER_HPP_
