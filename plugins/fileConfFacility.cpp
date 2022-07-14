/**
 * @file fileConfFacility.cpp file based implementataion of ConfFacility
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfFacility.hpp"
#include "appfwk/Issues.hpp"

#include "logging/Logging.hpp"

using namespace dunedaq::appfwk;

class fileConfFacility: public ConfFacility {

public:
    explicit fileConfFacility(std::string uri) : ConfFacility(uri) {
	// Allocate resources as needed
        auto col = uri.find_last_of(':');
        auto sep = uri.find("://");
        if (col == std::string::npos || sep == std::string::npos) { // assume filename
                m_dirname = m_conf_uri;
        } else {
                m_dirname = m_conf_uri.substr(sep+3);
        }

    } 

    nlohmann::json get_data(const std::string & app_name, const std::string & cmd) {
    	std::string fname = m_dirname + "/" + app_name + "_" + cmd + ".json"; 
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
private:
    std::string m_dirname;
} 

