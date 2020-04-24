/**
 * @file The FakeDataConsumerUserModule class interface
 *
 * FakeDataConsumerUserModule is a simple UserModule implementation that simply logs the fact that
 * it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef APP_FRAMEWORK_USERMODULES_FAKEDATACONSUMERUSERMODULE_HH
#define APP_FRAMEWORK_USERMODULES_FAKEDATACONSUMERUSERMODULE_HH

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/UserModules/UserModule.hh"
#include "app-framework-base/UserModules/UserModuleThreadHelper.hh"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace appframework {
/**
 * @brief FakeDataConsumerUserModule creates vectors of ints and sends them downstream
 */
class FakeDataConsumerUserModule : public UserModule {
   public:
    FakeDataConsumerUserModule(std::shared_ptr<BufferOutput<std::vector<int>>> inputBuffer, std::string id = "");

    std::future<std::string> execute_command(std::string cmd) override;

   private:
    // Commands
    std::string do_configure();
    std::string do_start();
    std::string do_stop();

    // Threading
    void do_work() ;
    UserModuleThreadHelper thread_;

    // Configuration (for validation)
    size_t nIntsPerVector_;
    int starting_int_;
    int ending_int_;
    std::string id_;
    std::string getId() { return id_ != "" ? id_ + ": " : ""; }
    std::shared_ptr<BufferOutput<std::vector<int>>> inputBuffer_;
};
}  // namespace appframework

#endif  // APP_FRAMEWORK_USERMODULES_FAKEDATACONSUMERUSERMODULE_HH