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
#include <unistd.h>

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

template <typename DATA_TYPE>
appframework::FanOutUserModule<DATA_TYPE>::FanOutUserModule(
    std::shared_ptr<BufferOutput<DATA_TYPE>> inputBuffer,
    std::initializer_list<std::shared_ptr<BufferInput<DATA_TYPE>>> outputBuffers)
    : SinkUserModule<DATA_TYPE>(inputBuffer),
      ThreadedUserModule(),
      mode_(FanOutMode::NotConfigured),
      outputBuffers_(outputBuffers) {}

template <typename DATA_TYPE>
std::future<std::string> appframework::FanOutUserModule<DATA_TYPE>::execute_command(std::string cmd) {
    if (cmd == "configure" || cmd == "Configure") {
        return std::async(std::launch::async, [&] { return do_configure(); });
    }
    if (cmd == "start" || cmd == "Start") {
        return std::async(std::launch::async, [&] { return do_start(); });
    }
    if (cmd == "stop" || cmd == "Stop") {
        return std::async(std::launch::async, [&] { return do_stop(); });
    }

    return std::async(std::launch::async, [] { return std::string("Unrecognized Command"); });
}

template <typename DATA_TYPE>
std::string appframework::FanOutUserModule<DATA_TYPE>::do_configure() {
    // TODO: Get configuration from ConfigurationManager!
    mode_ = FanOutMode::RoundRobin;
    wait_interval_us_ = 1000000;

    return "Success";
}

template <typename DATA_TYPE>
std::string appframework::FanOutUserModule<DATA_TYPE>::do_start() {
    start_working_thread_();
    return "Success";
}

template <typename DATA_TYPE>
std::string appframework::FanOutUserModule<DATA_TYPE>::do_stop() {
    stop_working_thread_();
    return "Success";
}

template <typename DATA_TYPE>
void appframework::FanOutUserModule<DATA_TYPE>::do_work() {
    auto roundRobinNext = outputBuffers_.begin();

    while (thread_started_.load()) {
        if (!SinkUserModule<DATA_TYPE>::inputBuffer_->empty()) {
            auto data = SinkUserModule<DATA_TYPE>::inputBuffer_->pop();

            if (mode_ == FanOutMode::Broadcast) {
                for (auto& o : outputBuffers_) {
                    o->push(std::vector<int>(data));
                }
            } else if (mode_ == FanOutMode::FirstAvailable) {
                auto sent = false;
                while (!sent) {
                    for (auto& o : outputBuffers_) {
                        if (!o->full()) {
                            o->push(std::move(data));
                            sent = true;
                            break;
                        }
                    }
                    if (!sent) {
                        usleep(wait_interval_us_);
                    }
                }
            } else if (mode_ == FanOutMode::RoundRobin) {
                while (true) {
                    if (!(*roundRobinNext)->full()) {
                        (*roundRobinNext)->push(std::move(data));

                        if (++roundRobinNext == outputBuffers_.end()) roundRobinNext = outputBuffers_.begin();
                        break;
                    } else {
                        usleep(wait_interval_us_);
                    }
                }
            }
        } else {
            usleep(wait_interval_us_);
        }
    }
}
#endif  // APP_FRAMEWORK_USERMODULES_FANOUTUSERMODULE_HH