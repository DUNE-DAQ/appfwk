/**
 * @file Queue class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUEI_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUEI_HH_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "cetlib/BasicPluginFactory.h"
#include "cetlib/compiler_macros.h"

#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                            \
  extern "C"                                                                   \
  {
#endif

#define DEFINE_DUNE_QUEUE(klass)                                              \
  EXTERN_C_FUNC_DECLARE_START                                                  \
  std::shared_ptr<appframework::QueueI> make()                                 \
  {                                                                            \
    return std::shared_ptr<appframework::QueueI>(new klass());                 \
  }                                                                            \
  }

namespace appframework {

/**
 * @brief Implementations of the Queue class are responsible for relaying data
 * between DAQModules within a DAQ Application
 *
 * Note that while the Queue class itself is not templated on a data type (so
 * it can be included in generic containers), all implementations should be.
 */
class QueueI
{

public:
  QueueI() {}

  virtual void Configure(const std::vector<std::string>& args = {}) {
  } ///< called to specify particular configuration

  QueueI(const QueueI&) = delete;
  QueueI& operator=(const QueueI&) = delete;
  QueueI(QueueI&&) = default;
  QueueI& operator=(QueueI&&) = default;
};

inline std::shared_ptr<QueueI>
makeQueue(std::string const& queue_name)
{
  static cet::BasicPluginFactory bpf("duneQueue", "make");

  return bpf.makePlugin<std::shared_ptr<QueueI>>(queue_name);
}

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_QUEUES_QUEUEI_HH_