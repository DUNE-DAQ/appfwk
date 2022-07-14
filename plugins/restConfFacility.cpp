/**
 * @file RestCommandFacility.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/CoonfFacility.hpp"
#include "appfwk/Issues.hpp"

#include <logging/Logging.hpp>
#include <cetlib/BasicPluginFactory.h>

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>

#include <fstream>
#include <string>
#include <memory>
#include <chrono>
#include <functional>

using namespace dunedaq::appfwk;

class restConfFacility : public ConfFacility {

public:
    // friend backend for calling functions on CF
    friend class RestEndpoint;

    explicit restConfFacility(std::string uri) : ConfFacility(uri) {

      m_uri = uri;
      // Parse URI
      auto col = uri.find_last_of(':');
      auto at  = uri.find('@');
      auto sep = uri.find("://");
      if (col == std::string::npos || sep == std::string::npos) { // enforce URI
        throw dunedaq::appfwk::MalformedUri(ERS_HERE, "Malformed URI: ", uri);
      }
      std::string scheme = uri.substr(0, sep);
      std::string iname = uri.substr(sep+3);
      if (iname.empty()) {
        throw dunedaq::appfwk::MalformedUri(ERS_HERE, "Missing interface name in ", uri);
      }
      std::string portstr = uri.substr(col+1);
      if (portstr.empty() || portstr.find(iname) != std::string::npos ) {
        throw dunedaq::appfwk::MalformedUri(ERS_HERE, "Can't bind without port in ", uri);
      }
      std::string epname = uri.substr(sep+3, at-(sep+3));
      std::string hostname = uri.substr(at+1, col-(at+1));

      int port = -1; 
      try { // to parse port
        port = std::stoi(portstr);
        if (!(0 <= port && port <= 65535)) { // valid port
          throw dunedaq::appfwk::MalformedUri(ERS_HERE, "Invalid port ", portstr); 
        }
      }
      catch (const std::exception& ex) {
        throw dunedaq::appfwk::MalformedUri(ERS_HERE, ex.what(), portstr);
      }

      try { // to setup backend
        command_executor_ = std::bind(&inherited::execute_command, this, std::placeholders::_1, std::placeholders::_2);
        rest_endpoint_= std::make_unique<dunedaq::restcmd::RestEndpoint>(hostname, port, command_executor_);
        rest_endpoint_->init(1); // 1 thread
        TLOG() <<"Endpoint open on: " << epname << " host:" << hostname << " port:" << portstr;
      } 
      catch (const std::exception& ex) {
         ers::error(dunedaq::appfwk::ConfFacilityInitialization(ERS_HERE, ex.what())); 
      }
    }

    nlohmann::json get_data(const std::string & app_name, const std::string & cmd, std::string & uri="") {
	    if (uri.empty()) 
		    uri = m_uri;

	    Http::Experimental::Client client;
	    auto opts = Http::Experimental::Client::options().threads(1).keepAlive(false).maxConnectionsPerHost(8);
    	    client.init(opts);

	    auto resp = client.get(uri).send();
	    nlohmann::json data;
	    resp.then(
            [&](Http::Response response) {
                std::cout << "Response code = " << response.code() << std::endl;
                data = response.body();
                if (!body.empty())
                    std::cout << "Response body = " << body << std::endl;
            },
            [&](std::exception_ptr exc) {
	        throw dunedaq::appfwk::ConfigurationRetreival(ERS_HERE, exc.what());
            });
	    nlohmann::json cmddata = data;
     return cmddata;
    }

private:
    std::string m_uri; 
};

extern "C" {
    std::shared_ptr<dunedaq::appfwk::ConfFacility> make(std::string uri) { 
        return std::shared_ptr<dunedaq::appfwk::ConfFacility>(new restConfFacility(uri));
    }
}
