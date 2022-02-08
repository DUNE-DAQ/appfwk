/**
 * @file QueueRegistry.hpp
 *
 * The QueueRegistry class declarations
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_QUEUEREGISTRY_HPP_
#define APPFWK_INCLUDE_APPFWK_QUEUEREGISTRY_HPP_

#include "appfwk/Queue.hpp"

#include "appfwk/app/Nljs.hpp"
#include "ers/Issue.hpp"
#include "opmonlib/InfoCollector.hpp"

#include <map>
#include <memory>
#include <string>

namespace dunedaq {
namespace appfwk {

/**
 * @brief The QueueRegistry class manages all Queue instances and gives out
 * handles to the Queues upon request
 */
class QueueRegistry
{
public:
  /**
   * @brief QueueRegistry destructor
   */
  ~QueueRegistry() = default;

  /**
   * @brief Get a handle to the QueueRegistry
   * @return QueueRegistry handle
   */
  static QueueRegistry& get();

  /**
   * @brief Get a handle to a Queue
   * @tparam T Type of the data stored in the Queue
   * @param name Name of the Queue
   * @return std::shared_ptr to generic queue pointer
   */
  template<typename T>
  std::shared_ptr<Queue<T>> get_queue(const std::string& name);

  /**
   * @brief Configure the QueueRegistry
   * @param qspecs Queue Specifications
   */
  void configure(const app::QueueSpecs& qspecs);

  // Gather statistics from queues
  void gather_stats(opmonlib::InfoCollector& ic, int level);

  // ONLY TO BE USED FOR TESTING!
  static void reset() { s_instance.reset(nullptr); }

  bool has_queue(std::string const& name) const { return m_queue_registry.count(name); }

private:
  struct QueueEntry
  {
    const std::type_info* m_type;
    std::shared_ptr<QueueBase> m_instance;
  };

  QueueRegistry() = default;

  template<typename T>
  std::shared_ptr<QueueBase> create_queue(const std::string& name, const app::QueueSpec& config);

  std::map<std::string, QueueEntry> m_queue_registry;
  std::map<std::string, app::QueueSpec> m_queue_config_map;

  bool m_configured{ false };

  static std::unique_ptr<QueueRegistry> s_instance;

  QueueRegistry(const QueueRegistry&) = delete;
  QueueRegistry& operator=(const QueueRegistry&) = delete;
  QueueRegistry(QueueRegistry&&) = delete;
  QueueRegistry& operator=(QueueRegistry&&) = delete;
};

} // namespace appfwk

// Disable coverage collection LCOV_EXCL_START
/**
 * @brief QueueTypeMismatch ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,            // namespace
                  QueueTypeMismatch, // issue class name
                  "Requested queue \"" << queue_name << "\" of type '" << target_type << "' already declared as type '"
                                       << source_type << "'", // message
                  ((std::string)queue_name)((std::string)source_type)((std::string)target_type))

/**
 * @brief QueueKindUnknown ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,           // namespace
                  QueueKindUnknown, // issue class name
                  "Queue kind \"" << queue_kind << "\" is unknown ",
                  ((std::string)queue_kind))

/**
 * @brief QueueNotFound ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,        // namespace
                  QueueNotFound, // issue class name
                  "Requested queue \"" << queue_name << "\" of type '" << target_type
                                       << "' could not be found.", // message
                  ((std::string)queue_name)((std::string)target_type))

/**
 * @brief QueueRegistryConfigured ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,                  // namespace
                  QueueRegistryConfigured, // issue class name
                  "QueueRegistry already configured",
                  ERS_EMPTY)

// Re-enable coverage collection LCOV_EXCL_STOP
} // namespace dunedaq

#include "detail/QueueRegistry.hxx"

#endif // APPFWK_INCLUDE_APPFWK_QUEUEREGISTRY_HPP_
