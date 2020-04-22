/**
 * @file The ThreadedUserModule class interface
 *
 * ThreadedUserModule builds upon the UserModule interface by defining a std::thread which runs the do_work() function
 * as well as methods to start and stop that thread.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_USERMODULES_THREADEDUSERMODULE_HH
#define APP_FRAMEWORK_BASE_USERMODULES_THREADEDUSERMODULE_HH

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <future>
#include <list>
#include <memory>
#include <string>

namespace appframework {
/**
 * @brief ThreadedUserModule contains a thread which runs the do_work() function
 */
class ThreadedUserModule : virtual public UserModule {
   public:
    /**
     * @brief ThreadedUserModule Constructor
     *
     * This constructor sets the defaults for the thread control variables
     */
    ThreadedUserModule() : thread_started_(false), working_thread_(nullptr) {}

   protected:
    /**
     * @brief Start the working thread (which executes the do_work() function)
     * @throws std::runtime_error if the thread is already running
     */
    void start_working_thread_() {
        if (thread_started_ || working_thread_ != nullptr) {
            throw std::runtime_error("Attempted to start UserModule working thread when it is already running!");
        }
        thread_started_ = true;
        working_thread_.reset(new std::thread([&] { do_work(); }));
    }
    /**
     * @brief Stop the working thread
     * @throws std::runtime_error If the thread has not yet been started
     * @throws std::runtime_error If the thread is not in the joinable state
     */
    void stop_working_thread_() {
        if (!thread_started_ || working_thread_ == nullptr) {
            throw std::runtime_error("Attempted to stop UserModule working thread when it is not running!");
        }
        thread_started_ = false;
        if (working_thread_->joinable()) {
            working_thread_->join();
        }
        throw std::runtime_error("Thread not in joinable state during UserModule working thread stop!");
    }

    /**
     * @brief This method defines the work that should be performed on the ThreadedUserModule working thread
     */
    virtual void do_work() = 0;

    std::atomic<bool>
        thread_started_;  ///< Variable which indicates whether the thread has been started and not stopped

   private:
    std::unique_ptr<std::thread> working_thread_;
};
}  // namespace appframework

#endif  // APP_FRAMEWORK_BASE_USERMODULES_THREADEDUSERMODULE_HH