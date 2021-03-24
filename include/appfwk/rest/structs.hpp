/**
 * @file structs.hpp RESTful API request and response body types
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef CMDLIB_INCLUDE_REST_API_STRUCTS_HPP_
#define CMDLIB_INCLUDE_REST_API_STRUCTS_HPP_

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace dunedaq::rest {

struct HealthZReply {
  std::string name;
  std::string state;
};

// to_json and from_json functions must be inline
// https://github.com/nlohmann/json/issues/542#issuecomment-290665546
inline void to_json(json& j, const HealthZReply& p) {
  j = json{{"name", p.name}, {"state", p.state}};
}

inline void from_json(const json& j, HealthZReply& p) {
  j.at("name").get_to(p.name);
  j.at("state").get_to(p.state);
}

} // namespace dunedaq::rest
#endif // CMDLIB_INCLUDE_REST_API_STRUCTS_HPP_