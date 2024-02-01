/**
 * @file ConfigurationManager.cpp ConfigurationManager class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/Session.hpp"
#include "oksdbinterfaces/Configuration.hpp"

using namespace dunedaq::appfwk;
using boost::property_tree::ptree;

ConfigurationManager::ConfigurationManager(std::string& config_spec, std::string& app_name, std::string& session_name)
{
  TLOG() << "configSpec <" << config_spec << "> session name " << session_name << " application name " << app_name;

  m_oks_config_spec = config_spec;
  m_app_name = app_name;
  m_session_name = session_name;

  m_confdb.reset(new oksdbinterfaces::Configuration(config_spec));

  TLOG_DBG(5) << "getting session";
  m_session = m_confdb->get<coredal::Session>(session_name);
  if (m_session == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get session";
    exit(0);
  }

  TLOG_DBG(5) << "getting app";
  m_application = m_confdb->get<coredal::Application>(app_name);
  if (m_application == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get app";
    exit(0);
  }
}

nlohmann::json ConfigurationManager::get_json_config(const std::string& class_name,
                                    const std::string& uid,
                                    bool direct_only) {
  using nlohmann::json;
  using namespace oksdbinterfaces;
  TLOG_DBG(9) << "Getting attributes for " << uid << " of class " << class_name;
  json attributes;
  auto class_info = m_confdb->get_class_info(class_name);
  ConfigObject obj;
  m_confdb->get(class_name, uid, obj);
  for (auto attr : class_info.p_attributes) {
    if (attr.p_type == type_t::u8_type) {
      add_json_value<uint8_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::u16_type) {
      add_json_value<uint16_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::u32_type) {
      add_json_value<uint32_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::u64_type) {
      add_json_value<uint64_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s8_type) {
      add_json_value<int8_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s16_type) {
      add_json_value<int16_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s32_type ||
             attr.p_type == type_t::s16_type) {
      add_json_value<int32_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s64_type) {
      add_json_value<int64_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::float_type) {
      add_json_value<float>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::double_type) {
      add_json_value<double>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::bool_type) {
      add_json_value<bool>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if ((attr.p_type == type_t::string_type) ||
             (attr.p_type == type_t::enum_type) ||
             (attr.p_type == type_t::date_type) ||
             (attr.p_type == type_t::time_type)) {
      add_json_value<std::string>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
  }
  if (!direct_only) {
    TLOG_DBG(9) << "Processing  relationships";
    for (auto iter: class_info.p_relationships) {
      std::string rel_name = iter.p_name;
      if (iter.p_cardinality == cardinality_t::zero_or_one ||
          iter.p_cardinality == cardinality_t::only_one) {
        ConfigObject rel_obj;
        obj.get(rel_name, rel_obj);
        if (!rel_obj.is_null()) {
          TLOG_DBG(9) << "Getting attibute of relationship " << rel_name;
          attributes[rel_name] = get_json_config(rel_obj.class_name(),
                                                 rel_obj.UID());
        }
        else {
          TLOG_DBG(9) << "Relationship " << rel_name << " not set";
        }
      }
      else {
        TLOG_DBG(9) << "Relationship " << rel_name << " is multi value. "
                    << "Getting attibutes for relationship.";
        std::vector<ConfigObject> rel_vec;
        obj.get(rel_name, rel_vec);
        std::vector<json> configs;
        for (auto rel_obj : rel_vec) {
          TLOG_DBG(9) << "Getting attibute of relationship " << rel_obj.UID();
          auto rel_conf = get_json_config(rel_obj.class_name(), rel_obj.UID());
          configs.push_back(rel_conf);
        }
        attributes[rel_name] = configs;
      }
    }
  }
  json json_config;
  json_config[uid] = attributes;
  return json_config;
}
