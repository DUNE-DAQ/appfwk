/**
 * @file StateObject.hpp Class that describes an object with a state
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef CMDLIB_INCLUDE_APPFWK_STATEOBJECT_HPP_
#define CMDLIB_INCLUDE_APPFWK_STATEOBJECT_HPP_

#include <mutex>
#include <string>

namespace dunedaq::appfwk {

/**
 * @brief Interface needed by commanded objects in the DAQ
 */
class StateObject
{
public:

  /**
   * @brief StateObject Constructor
   * @param initial_state The initial state of this object
   */
  StateObject(const std::string& initial_state): m_state(initial_state)
  {}

  StateObject(StateObject const&) = delete;            ///< StateObject is not copy-constructible
  StateObject(StateObject&&) = default;                ///< StateObject is move-constructible
  StateObject& operator=(StateObject const&) = delete; ///< StateObject is not copy-assignable
  StateObject& operator=(StateObject&&) = default;     ///< StateObject is move-assignable
  virtual ~StateObject() = default;                    ///< Default virtual destructor

  /**
   * Get state of this object.
   * Thread safe
   */
  void  set_state(std::string s) {
     const std::lock_guard<std::mutex> lock(m_state_mutex);
     m_state = s;
  } 
  /**
   * Set new state on this object.
   * Thread safe
   */
  std::string get_state() {
    const std::lock_guard<std::mutex> lock(m_state_mutex);
    return m_state ;
  }
private:
  std::mutex m_state_mutex;
  std::string m_state;
};

} // namespace dunedaq::appfwk

#endif // CMDLIB_INCLUDE_APPFWK_STATEOBJECT_HPP__
