/**
 * @file Api.cpp RESTful API for DAQ Applications
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/rest/Api.hpp"
#include "appfwk/DAQModuleManager.hpp"
#include "appfwk/DAQModule.hpp"
#include "cmdlib/CommandedObject.hpp"
#include "logging/Logging.hpp"

#include <stdint.h>

using json = nlohmann::json;

namespace dunedaq {
namespace rest {

Api::Api(std::uint16_t port)
  : m_port(port), m_address{ Pistache::Ipv4::any(), m_port }
  , m_http_endpoint{ std::make_shared<Pistache::Http::Endpoint>( m_address ) }
  , m_started(0), m_start_failed(0)
{
  auto opts = Pistache::Http::Endpoint::options()
    .threads(1)
    .maxRequestSize(15728640) // 15MB
    .maxResponseSize(1048576) // 1MB 
    .flags(Pistache::Tcp::Options::ReuseAddr)
    .flags(Pistache::Tcp::Options::ReusePort);
  m_http_endpoint->init(opts);

  register_get("/", Pistache::Rest::Routes::bind(&Api::handle_get_root, this));
  // healthz will always respond
  // even if register_zpages() has not been called
  register_get("/api/v0/healthz", Pistache::Rest::Routes::bind(&Api::handle_get_healthz, this));
  register_get("/api/v0/modules", Pistache::Rest::Routes::bind(&Api::handle_get_modules, this));
  register_get("/api/v0/commands/history", Pistache::Rest::Routes::bind(&Api::handle_get_commands_history, this));
}

Api::~Api()
{
  stop();
}

void
Api::register_zpages(std::string name, appfwk::StateObject* stated)
{
  m_app_name = name;
  m_state_obj = stated;
}

void
Api::register_modulemanager(appfwk::DAQModuleManager* mm)
{
  m_module_manager = mm;
}

void
Api::start()
{
  if (m_started) {
    throw RestApiAlreadyRunning(ERS_HERE, m_port);
  }
  // any routes added after m_router.handler() will not work
  m_http_endpoint->setHandler(m_router.handler());
  m_server_thread = std::thread(&Api::serve, this);
  // determine the actually open endpoint port
  // this will always equal m_port except when m_port = 0
  auto realPort = m_http_endpoint->getPort();
  while (realPort == 0 && !m_start_failed)
  {
    // unfortunatuly we can't seem to open the listener first 
    // and _then_ do the blocking call to pistache's .serve()
    usleep(100);
    realPort = m_http_endpoint->getPort();
  }
  m_port = realPort;
  // wait for api to be started
  do
  {
    usleep(100);
  } while (!m_started && !m_start_failed);
  usleep(100); // for good measure...

  // throw if serve() instantly failed
  // e.g. can't open port
  if (m_start_failed) {
    throw RestApiStartFailed(ERS_HERE, m_port);
  }
  
  ers::info(RestApiStarted(ERS_HERE, realPort));
}

void
Api::serve()
{
  try {
    m_started = 1;
    m_http_endpoint->serve(); // blocking call
  }
  catch(std::exception& ex) {
    m_start_failed = 1;
    ers::error(RestApiStartFailed(ERS_HERE, m_port, ex));
    m_http_endpoint->shutdown();
  }
}

void
Api::stop()
{
  m_http_endpoint->shutdown();
  if (m_server_thread.joinable()) {
    m_server_thread.join();
  }
  m_started = 0;
  m_start_failed = 0;
}

void
Api::handle_get_root([[maybe_unused]] const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  json j;
  j["routes"] = m_routes;
  response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
  response.send(Pistache::Http::Code::Ok, j.dump(2));
}

void
Api::handle_get_modules([[maybe_unused]] const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  if (!m_module_manager) {
    response.send(Pistache::Http::Code::Service_Unavailable, "no module manager to query");
    return;
  }

  json j;
  j["modules"] = json::array();
  for (const auto& [module_name, module_ptr] : m_module_manager->get_modules()) {
    json j_item;
    j_item["name"] = module_name;
    j_item["commands"] = module_ptr->get_commands();
    j["modules"].push_back(j_item);
  }

  response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
  response.send(Pistache::Http::Code::Ok, j.dump(2));
}

void
Api::handle_get_commands_history([[maybe_unused]] const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  if (!m_module_manager) {
    response.send(Pistache::Http::Code::Service_Unavailable, "no module manager to query");
    return;
  }

  json j;
  j["commands"] = m_module_manager->gather_history();

  response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
  response.send(Pistache::Http::Code::Ok, j.dump(2));
}

void
Api::handle_get_healthz([[maybe_unused]] const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  auto c = Pistache::Http::Code::Ok;
  HealthZReply result;

  // if the name is not set, we're not healthy
  if (m_app_name.empty()) {
    c = Pistache::Http::Code::Service_Unavailable;
  }
  result.name = m_app_name;

  // if there is no StateObject to query state from, we're not healthy
  if (m_state_obj) {
    result.state = m_state_obj->get_state();
  } else {
    c = Pistache::Http::Code::Service_Unavailable;
  }

  // if there is no state or state is NONE, we're not healthy
  if (result.state.empty() || result.state == "NONE") {
    c = Pistache::Http::Code::Service_Unavailable;
  }

  // convert to json
  json j = result;
  
  // send json
  response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
  response.send(c, j.dump(2));
}

void
Api::register_get(const std::string &resource, Pistache::Rest::Route::Handler handler)
{
  Pistache::Rest::Routes::Get(m_router, resource, handler);
  m_routes.insert("GET " + resource);
}

void
Api::register_post(const std::string &resource, Pistache::Rest::Route::Handler handler)
{
  Pistache::Rest::Routes::Post(m_router, resource, handler);
  m_routes.insert("POST " + resource);
}

void
Api::register_put(const std::string &resource, Pistache::Rest::Route::Handler handler)
{
  Pistache::Rest::Routes::Put(m_router, resource, handler);
  m_routes.insert("PUT " + resource);
}

void
Api::register_patch(const std::string &resource, Pistache::Rest::Route::Handler handler)
{
  Pistache::Rest::Routes::Patch(m_router, resource, handler);
  m_routes.insert("PATCH " + resource);
}

void
Api::register_delete(const std::string &resource, Pistache::Rest::Route::Handler handler)
{
  Pistache::Rest::Routes::Delete(m_router, resource, handler);
  m_routes.insert("DELETE " + resource);
}

void
Api::register_options(const std::string &resource, Pistache::Rest::Route::Handler handler)
{
  Pistache::Rest::Routes::Options(m_router, resource, handler);
  m_routes.insert("OPTIONS " + resource);
}

void
Api::register_head(const std::string &resource, Pistache::Rest::Route::Handler handler)
{
  Pistache::Rest::Routes::Head(m_router, resource, handler);
  m_routes.insert("HEAD " + resource);
}

void
Api::unregister(Pistache::Http::Method method, const std::string &resource)
{
  Pistache::Rest::Routes::Remove(m_router, method, resource);
  m_routes.erase(std::string(Pistache::Http::methodString(method)) + " " + resource);
}


} // namespace rest
} // namespace dunedaq