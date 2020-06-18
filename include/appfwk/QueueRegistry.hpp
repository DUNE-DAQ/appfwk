/**
 * @file QueueRegistry.hpp
 *
 * The QueueRegistry class declarations
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HPP_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HPP_

#include "appfwk/Queue.hpp"

#include "ers/Issue.h"
#include <map>
#include <memory>
#include <string>

namespace dunedaq {
namespace appfwk {

/**
 * @brief The QueueConfig class encapsulates the basic configuration common to
 * all Queue types
 */
struct QueueConfig
{
  /**
   * @brief Enumeration of all possible types of Queue
   */
  enum queue_kind
  {
    kUnknown = -1,
    kStdDeQueue = 1 ///< The StdDeQueue
  };

  /**
   * @brief  Transform a string to a queue_kind
   * @param name Name of the Queue Type
   * @return Queue type corresponding to the name. Currently only std_deque.
   */
  static queue_kind stoqk(const std::string& name);

  QueueConfig::queue_kind kind = queue_kind::kUnknown; ///< The kind of Queue represented by this
                                                       ///< QueueConfig
  size_t size = 0;                                     ///< The size of the queue
};

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
  ~QueueRegistry();

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
   * @param configmap Map relating Queue names to their configurations
   */
  void configure(const std::map<std::string, QueueConfig>& configmap);

private:
  struct QueueEntry
  {
    const std::type_info* type;
    std::shared_ptr<NamedObject> instance;
  };

  QueueRegistry();

  template<typename T>
  std::shared_ptr<NamedObject> create_queue(std::string name, const QueueConfig& config);

  std::map<std::string, QueueEntry> queue_registry_;
  std::map<std::string, QueueConfig> queue_configmap_;

  bool configured_;

  static QueueRegistry* me_;

  QueueRegistry(const QueueRegistry&) = delete;
  QueueRegistry& operator=(const QueueRegistry&) = delete;
  QueueRegistry(QueueRegistry&&) = delete;
  QueueRegistry& operator=(QueueRegistry&&) = delete;
};

} // namespace appfwk

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
ERS_DECLARE_ISSUE(appfwk,           // namespace
                  QueueNotFound, // issue class name
                  "Requested queue \"" << queue_name << "\" of type '" << target_type << "' could not be found.", // message
                  ((std::string)queue_name)((std::string)target_type))

/**
 * @brief QueueRegistryConfigured ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,        // namespace
                  QueueRegistryConfigured, // issue class name
                  "QueueRegistry already configured",ERS_EMPTY)

} // namespace dunedaq

#include "detail/QueueRegistry.hxx"

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUEUEREGISTRY_HPP_
