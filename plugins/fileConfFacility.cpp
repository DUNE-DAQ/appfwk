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

#include <fstream>

using namespace dunedaq::appfwk;

class fileConfFacility: public ConfFacility {

public:
    explicit fileConfFacility(std::string uri) : ConfFacility(uri) {
	m_uri = uri;
    }

    nlohmann::json get_data(const std::string & app_name, const std::string & cmd, const std::string & uri) {
        if(!uri.empty())
		m_uri = uri;

        auto sep = m_uri.find("://");
	std::string dirname;
        if (sep == std::string::npos) { // bad URI!
		throw InvalidConfigurationURI(ERS_HERE, uri);
        } else {
                dirname = m_uri.substr(sep+3);
        }

	std::string fname = dirname + "/" + app_name + "_" + cmd + ".json";
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
        TLOG(10) << app_name << " received " << cmd << " : " << data;
    	return data;
    }
protected:
    typedef ConfFacility inherited;

private:
    std::string m_uri;
};

extern "C" {
    std::shared_ptr<dunedaq::appfwk::ConfFacility> make(std::string uri) {
        return std::shared_ptr<dunedaq::appfwk::ConfFacility>(new fileConfFacility(uri));
    }
}
