/**
 * @file QueueBase.hpp
 *
 * This is the base class for Queue objects which connect DAQModules. Queues
 * are exposed to DAQModules via the DAQSource and DAQSink classes, and should
 * not be handled directly. Queues are registered with QueueRegistry for
 * retrieval by DAQSink and DAQSource instances. QueueBase allows to expose common
 * behavior of queues irrespective of their type.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_QUEUEBASE_HPP_
#define APPFWK_INCLUDE_APPFWK_QUEUEBASE_HPP_

#include "appfwk/NamedObject.hpp"
#include "appfwk/queueinfo/Nljs.hpp"

#include "opmonlib/InfoCollector.hpp"

#include "ers/Issue.hpp"

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <atomic>

namespace dunedaq {
namespace appfwk {

/**
 * @brief The QueueBase class allows to address generic behavior of any Queue implementation
 *
 */
class QueueBase : public NamedObject
{
public:

  /**
   * @brief QueueBase Constructor
   * @param name Name of the Queue instance
   */
  explicit QueueBase(const std::string& name, size_t capacity)
    : NamedObject(name)
    , m_capacity(capacity)
    , m_num_elements(0)
  {}

  /**
   * @brief Method to retrieve information (occupancy) from
   * queues.
   */ 
  void get_info(opmonlib::InfoCollector& ci, int /*level*/) { 
    queueinfo::Info info ;
    info.capacity = this -> get_capacity() ;
    info.number_of_elements = this->get_num_elements() ;
    ci.add( info ) ;
  }

  /**
   * @brief Get the capacity (max size) of the queue
   * @return size_t capacity
   */ 
  size_t get_capacity() const { return m_capacity; }

  size_t get_num_elements() {return m_num_elements.load(std::memory_order_acquire); }

protected:
  
  void increase_num_elements() {++m_num_elements; }
  void decrease_num_elements() {--m_num_elements; }


private:
  QueueBase(const QueueBase&) = delete;
  QueueBase& operator=(const QueueBase&) = delete;
  QueueBase(QueueBase&&) = default;
  QueueBase& operator=(QueueBase&&) = default;

  size_t m_capacity;
  std::atomic<size_t> m_num_elements;
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_QUEUE_HPP_
