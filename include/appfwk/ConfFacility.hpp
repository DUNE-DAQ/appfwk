/**
 * @file ConfFacility.hpp ConfFacility base definitions
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef APPFWK_SRC_CONFFACILITY
#define APPFWK_SRC_CONFFACILITY

#include "Issues.hpp"

#include <cetlib/BasicPluginFactory.h>
#include <cetlib/compiler_macros.h>
#include <nlohmann/json.hpp>
#include <string>

#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                                                                    \
  extern "C"                                                                                                           \
  {
#endif

/**
 * @brief Declare the function that will be called by the plugin loader
 * @param klass Class to be defined as a DUNE Configuration Facility
 */
#define DEFINE_DUNE_CONFIGURATION_FACILITY(klass)                                                                            \
  EXTERN_C_FUNC_DECLARE_START                                                                                          \
  std::unique_ptr<dunedaq::appfwk::ConfFacility> make()                                                             \
  {                                                                                                                    \
    return std::unique_ptr<dunedaq::appfwk::ConfFacility>(new klass());                                             \
  }                                                                                                                    \
  } 

namespace dunedaq::appfwk {

/**
 * @brief Interface needed by DAQ apps and services for configuration handling
 */
class ConfFacility
{
public:
  explicit ConfFacility(std::string /*uri*/) {}
  ~ConfFacility();
  ConfFacility(const ConfFacility&) = 
    delete; ///< ConfFacility is not copy-constructible
  ConfFacility& operator=(const ConfFacility&) =
    delete; ///< ConfFacility is not copy-assignable
  ConfFacility(ConfFacility&&) =
    delete; ///< ConfFacility is not move-constructible
  ConfFacility& operator=(ConfFacility&&) =
    delete; ///< ConfFacility is not move-assignable


  virtual nlohmann::json get_data(const std::string& app_name, const std::string& cmd, const std::string& uri) = 0;

private:

};

std::shared_ptr<ConfFacility>
make_conf_facility(std::string const& uri)
{
  auto sep = uri.find("://");
  std::string scheme;
  if (sep == std::string::npos) {
	  ers::error(ConfFacilityCreationFailed(ERS_HERE, uri, "Invalid URI"));
      throw ConfFacilityCreationFailed(ERS_HERE, uri, "Invalid URI");
  } else { // with scheme
      scheme = uri.substr(0, sep);
  }
  std::string plugin_name = scheme + "ConfFacility";
  static cet::BasicPluginFactory bpf("duneConfFacility", "make");
  std::shared_ptr<ConfFacility> cf_ptr;
  try {
    cf_ptr = bpf.makePlugin<std::shared_ptr<ConfFacility>>(plugin_name, uri);
  } catch (const cet::exception &cexpt) {
    ers::error(ConfFacilityCreationFailed(ERS_HERE, uri, cexpt));
    throw ConfFacilityCreationFailed(ERS_HERE, uri, cexpt);
  } catch (const ers::Issue &iexpt) {
	  ers::error(ConfFacilityCreationFailed(ERS_HERE, uri, iexpt));
    throw ConfFacilityCreationFailed(ERS_HERE, uri, iexpt);
  } catch (...) {  // NOLINT JCF Jan-27-2021 violates letter of the law but not the spirit
          ers::error(ConfFacilityCreationFailed(ERS_HERE, uri));

      	  throw ConfFacilityCreationFailed(ERS_HERE, uri, "Unknown error.");
  }
  return cf_ptr;
}

} // namespace dunedaq::appfwk

#endif // APPFWK_SRC_CONFFACILITY
