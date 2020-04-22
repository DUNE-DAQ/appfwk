/**
 * @file simple_test_app to show basic functionality of DAQProcess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#include <memory>
#include <vector>

#include "app-framework/Buffers/DequeBuffer.icc"
#include "app-framework/DAQProcess.hh"
#include "app-framework/QueryResponseCommandFacility.hh"
#include "app-framework/UserModules/FakeDataConsumerUserModule.hh"
#include "app-framework/UserModules/FakeDataProducerUserModule.hh"
#include "app-framework/UserModules/FanOutUserModule.icc"

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ =
    std::unique_ptr<CommandFacility>(new QueryResponseCommandFacility());

class producer_consumer_test_app_ModuleList : public ModuleList {
    // Inherited via ModuleList
    virtual void ConstructGraph(BufferMap& buffer_map, UserModuleMap& user_module_map,
                                CommandOrderMap& command_order_map) override {
        std::shared_ptr<DequeBuffer<std::vector<int>>> producerToFanOut(new DequeBuffer<std::vector<int>>());
        std::shared_ptr<DequeBuffer<std::vector<int>>> fanOutToConsumer1(new DequeBuffer<std::vector<int>>());
        std::shared_ptr<DequeBuffer<std::vector<int>>> fanOutToConsumer2(new DequeBuffer<std::vector<int>>());
        buffer_map["producerToFanOut"] = producerToFanOut;
        buffer_map["fanOutToConsumer1"] = fanOutToConsumer1;
        buffer_map["fanOutToConsumer2"] = fanOutToConsumer2;

        user_module_map["producer"].reset(new FakeDataProducerUserModule(producerToFanOut));
        user_module_map["fanOut"].reset(
            new FanOutUserModule<std::vector<int>>(producerToFanOut, {fanOutToConsumer1, fanOutToConsumer2}));
        user_module_map["consumer1"].reset(new FakeDataConsumerUserModule(fanOutToConsumer1, "C1"));
        user_module_map["consumer2"].reset(new FakeDataConsumerUserModule(fanOutToConsumer2, "C2"));
    }
};
}  // namespace appframework

int main(int argc, char* argv[]) {
    std::list<std::string> args;
    for (int ii = 1; ii < argc; ++ii) {
        args.push_back(std::string(argv[ii]));
    }

    appframework::DAQProcess theDAQProcess(args);

    auto ml = std::unique_ptr<appframework::ModuleList>(new appframework::producer_consumer_test_app_ModuleList());
    theDAQProcess.register_modules(ml);

    return theDAQProcess.listen();
}