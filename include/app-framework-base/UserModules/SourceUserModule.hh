/**
 * @file The SourceUserModule class interface
 *
 * SourceUserModule extends the UserModule interface to define a UserModule which puts data into a BufferInput
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_USERMODULES_SOURCEUSERMODULE_HH
#define APP_FRAMEWORK_BASE_USERMODULES_SOURCEUSERMODULE_HH

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <future>
#include <list>
#include <memory>
#include <string>

namespace appframework {
/**
 * @brief SourceUserModule puts data into a BufferInput
 * @tparam DATA_TYPE The type of data expected by the Buffer
 */
template <typename DATA_TYPE>
class SourceUserModule : virtual public UserModule {
   public:
    /**
     * @brief SourceUserModule Constructor
     * @param outputBuffer The Buffer endpoint into which this UserModule will put data
     */
    SourceUserModule(std::shared_ptr<BufferInput<DATA_TYPE>> outputBuffer) : outputBuffer_(outputBuffer) {}

   protected:
    std::shared_ptr<BufferInput<DATA_TYPE>> outputBuffer_;  ///< Pointer to the Buffer endpoint
};
}  // namespace appframework

#endif  // APP_FRAMEWORK_BASE_USERMODULES_SOURCEUSERMODULE_HH