/**
 * @file The SinkUserModule class interface
 *
 * SinkUserModule extends the UserModule interface to define a UserModule which takes data from a BufferOutput
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_USERMODULES_SINKUSERMODULE_HH
#define APP_FRAMEWORK_BASE_USERMODULES_SINKUSERMODULE_HH

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <future>
#include <list>
#include <memory>
#include <string>

namespace appframework {
/**
 * @brief SinkUserModule takes data from a BufferOutput
 * @tparam DATA_TYPE The type of data expected from the Buffer
 */
template <typename DATA_TYPE>
class SinkUserModule : virtual public UserModule {
   public:
    /**
     * @brief SinkUserModule Constructor
     * @param inputBuffer The Buffer endpoint from which this UserModule will read data
     */
    SinkUserModule(std::shared_ptr<BufferOutput<DATA_TYPE>> inputBuffer) : inputBuffer_(inputBuffer) {}

   protected:
    std::shared_ptr<BufferOutput<DATA_TYPE>> inputBuffer_;  ///< Pointer to the Buffer endpoint
};
}  // namespace appframework

#endif  // APP_FRAMEWORK_BASE_USERMODULES_SINKUSERMODULE_HH