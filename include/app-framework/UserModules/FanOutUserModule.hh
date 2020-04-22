/**
 * @file The FanOutUserModule class interface
 *
 * FanOutUserModule is a simple UserModule implementation that simply logs the fact that
 * it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef APP_FRAMEWORK_USERMODULES_FANOUTUSERMODULE_HH
#define APP_FRAMEWORK_USERMODULES_FANOUTUSERMODULE_HH

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/UserModules/SinkUserModule.hh"
#include "app-framework-base/UserModules/ThreadedUserModule.hh"

#include <future>
#include <list>
#include <memory>
#include <string>

namespace appframework {
/**
 * @brief FanOutUserModule sends data to multiple Buffers
 */
template <typename DATA_TYPE>
class FanOutUserModule : public virtual SinkUserModule<DATA_TYPE>, public virtual ThreadedUserModule {
   public:
    FanOutUserModule(std::shared_ptr<BufferOutput<DATA_TYPE>> inputBuffer,
                     std::initializer_list<std::shared_ptr<BufferInput<DATA_TYPE>>> outputBuffers);

    /**
     * @brief Logs the reception of the command
     * @param cmd Command from DAQProcess
     */
    std::future<std::string> execute_command(std::string cmd) override;

    enum class FanOutMode { NotConfigured, Broadcast, RoundRobin, FirstAvailable };

   private:
    // Commands
    std::string do_configure();
    std::string do_start();
    std::string do_stop();

    // Threading
    void do_work() final;

    // Configuration
    FanOutMode mode_;
    std::list<std::shared_ptr<BufferInput<DATA_TYPE>>> outputBuffers_;
    size_t wait_interval_us_;
};
}  // namespace appframework

#endif  // APP_FRAMEWORK_USERMODULES_FANOUTUSERMODULE_HH