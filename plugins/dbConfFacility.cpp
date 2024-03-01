/**
 * @file dbCommandFacility.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfFacility.hpp"
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
using namespace Pistache;

class dbConfFacility : public ConfFacility {

public:

    explicit dbConfFacility(std::string uri) : ConfFacility(uri) {

      m_uri = uri;
      // Parse URI
      auto sep = uri.find("://");
      if (sep == std::string::npos) { // enforce URI
        throw dunedaq::appfwk::InvalidConfigurationURI(ERS_HERE, "Malformed URI: ", uri);
      }
      m_uri = "http" + uri.substr(sep);

    }

    nlohmann::json get_data(const std::string & app_name, const std::string & cmd, const std::string & uri) {
	    if (!uri.empty()) {
		auto sep = uri.find("://");
      		if (sep == std::string::npos) { // enforce URI
        		throw dunedaq::appfwk::InvalidConfigurationURI(ERS_HERE, "Malformed URI: ", uri);
      		}
      		m_uri = "http" + uri.substr(sep);
	    }

	    Http::Experimental::Client client;
	    auto opts = Http::Experimental::Client::options().threads(1).keepAlive(true).maxConnectionsPerHost(8);
    	    client.init(opts);

	    TLOG_DEBUG() << "HTTP client instanciated and options set " << m_uri;
	    auto resp = client.get(m_uri+"&app_name="+app_name+"&cmd_name="+cmd).send();
	    nlohmann::json data;
            std::vector<Async::Promise<Http::Response>> responses;

	    resp.then(
            [&](Http::Response response) {
              data = nlohmann::json::parse(response.body());
            },
            [&](std::exception_ptr e) {
	    	try {
                    std::rethrow_exception(e);
                } catch (const Pistache::Error &e) {
	            ers::error(dunedaq::appfwk::ConfigurationRetreival(ERS_HERE, m_uri, e.what()));
    		}
            });

	    responses.push_back(std::move(resp));
	    auto sync = Async::whenAll(responses.begin(), responses.end());
	    Async::Barrier<std::vector<Http::Response>> barrier(sync);
	    barrier.wait_for(std::chrono::seconds(5));
	    client.shutdown();

            TLOG_DEBUG(10) << app_name << " received " << cmd << " : " << data;
     	    return data;
    }
protected:
    typedef ConfFacility inherited;
private:
    std::string m_uri;
};

extern "C" {
    std::shared_ptr<dunedaq::appfwk::ConfFacility> make(std::string uri) {
        return std::shared_ptr<dunedaq::appfwk::ConfFacility>(new dbConfFacility(uri));
    }
}
