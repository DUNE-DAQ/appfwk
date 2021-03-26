/**
 * @file API.hpp RESTful API for DAQ Applications
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef CMDLIB_INCLUDE_REST_API_HPP_
#define CMDLIB_INCLUDE_REST_API_HPP_

#include "appfwk/NamedObject.hpp"
#include "appfwk/StateObject.hpp"
#include "appfwk/DAQModuleManager.hpp"
#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"
#include "structs.hpp"

#include <pistache/http.h>
#include <pistache/http_header.h>
#include <pistache/description.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/client.h>
#include <pistache/mime.h>

#include <thread>
#include <stdint.h>
#include <set>

namespace dunedaq {

ERS_DECLARE_ISSUE(rest,
                  RestApiStartFailed,
                  "REST API failed to start on port " << port,
                  ((std::uint16_t) port)
)
ERS_DECLARE_ISSUE(rest,
                  RestApiStarted,
                  "REST API listening on port " << port,
                  ((std::uint16_t) port)
)
ERS_DECLARE_ISSUE(rest,
                  RestApiAlreadyRunning,
                  "REST API already running on port " << port,
                  ((std::uint16_t) port)
)

namespace rest {

class Api
{
public:
  /**
   * Construct a new API on the given port.
   * The port is not opened at construction time.
   * @param port the port to listen on
   */
  Api(std::uint16_t /*port*/);
  ~Api();
  Api() = delete;                        ///< Api is not default-constructible
  Api(Api const&) = delete;              ///< Api is not copy-constructible
  Api(Api&&) = default;                  ///< Api is move-constructible
  Api& operator=(Api const&) = delete;   ///< Api is not copy-assignable
  Api& operator=(Api&&) = default;       ///< Api is move-assignable

  /**
   * @brief Register elements needed to serve z pages.
   */
  void register_zpages(std::string /*name*/, appfwk::StateObject*);

  /**
   * Register the DAQModuleManager that will be used
   * to query for module info.
   */
  void register_modulemanager(appfwk::DAQModuleManager*);

  /**
   * @brief Start this API.
   * 
   * Spawns a separate thread that will serve incoming connections.
   * 
   * @throws RestApiAlreadyRunning if start() was called and succeeded before
   * @throws RestApiStartFailed if the api failed to start
   */
  void start();

  /**
   * @brief Stops the API.
   */
  void stop();

  /**
   * Denotes whether this application has successfully started
   * after a call to start()
   */
  bool is_started() { return m_started; }
  /**
   * Denotes whether this application has failed to start
   * after a call to start()
   */
  bool start_failed() { return m_start_failed; }

  Pistache::Port get_port() { return m_port; }

  /**
   * @brief Add and remove routes.
   * These functions forward directly to the pistache router, and do some extra bookkeeping
   */
  void register_get(const std::string &resource, Pistache::Rest::Route::Handler handler);
  void register_post(const std::string &resource, Pistache::Rest::Route::Handler handler);
  void register_put(const std::string &resource, Pistache::Rest::Route::Handler handler);
  void register_patch(const std::string &resource, Pistache::Rest::Route::Handler handler);
  void register_delete(const std::string &resource, Pistache::Rest::Route::Handler handler);
  void register_options(const std::string &resource, Pistache::Rest::Route::Handler handler);
  void register_head(const std::string &resource, Pistache::Rest::Route::Handler handler);
  void unregister(Pistache::Http::Method method, const std::string &resource);

private:
  // The name of the DAQ Application this API runs for
  std::string m_app_name;
  // The StateObject this API will call get_state() on
  appfwk::StateObject* m_state_obj;
  // The DAQModuleManager the API will use to serve /modules
  appfwk::DAQModuleManager* m_module_manager;
  // The port number this API will serve requests on
  Pistache::Port m_port;
  // The address this API will serve requests on
  Pistache::Address m_address;
  // Wether this API already successfully started
  bool m_started;
  // Wether the last start() produced an error
  bool m_start_failed;
  // Pistache endpoint that wraps m_address
  std::shared_ptr<Pistache::Http::Endpoint> m_http_endpoint;
  // Pistache router
  Pistache::Rest::Router m_router;
  // Internal list of registered routes in m_router
  std::set<std::string> m_routes;
  // Thread that runs the API server
  std::thread m_server_thread;

  /**
   * @brief Function handling 'GET /'
   */
  void handle_get_root(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);
  /**
   * @brief Function handling 'GET /api/v0/healthz'
   */
  void handle_get_healthz(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);
  /**
   * @brief Function handling 'GET /api/v0/modules'
   */
  void handle_get_modules(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);
  /**
   * @brief Function handling 'GET /api/v0/commands/history'
   */
  void handle_get_commands_history(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);

  /**
   * Blocking call that starts the API and serves requests.
   * Used by start()
   * 
   * @throws RestApiStartFailed if the api failed to start
   */
  void serve(); 
};

} // namespace rest
} // namespace dunedaq
#endif // CMDLIB_INCLUDE_REST_API_HPP_