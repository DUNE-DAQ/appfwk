/**
 * @file ConfigurationWrapper.hxx ConfigurationWrapper implementataion
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "cmdlib/cmd/Nljs.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "logging/Logging.hpp"


namespace dunedaq {
namespace appfwk {

ConfigurationWrapper::ConfigurationWrapper(const std::string & conf_uri, const std::string & name)
  : m_conf_uri(conf_uri), m_app_name(name)
{
}

nlohmann::json ConfigurationWrapper::get_data(const std::string & cmd) {
    // Allocate resources as needed
    auto col = m_conf_uri.find_last_of(':');
    auto sep = m_conf_uri.find("://");
    std::string dirname;
    if (col == std::string::npos || sep == std::string::npos) { // assume filename
      dirname = m_conf_uri;
    } else {
      dirname = m_conf_uri.substr(sep+3);
    }

    std::string fname = dirname + "/" + m_app_name + "_" + cmd + ".json"; 
    TLOG() <<"Loading parameters from file: " << fname;
   
    std::ifstream ifs;
    ifs.open(fname, std::fstream::in);
    if (!ifs.is_open()) {
      throw BadFile(ERS_HERE, fname);
    } 

    nlohmann::json data;
    try {
      data = nlohmann::json::parse(ifs);
    } catch (const std::exception& ex) {
      throw CannotParseData(ERS_HERE, ex.what());
    }
    return data;
}

} // namespace appfwk
} // namespace dunedaq
 

