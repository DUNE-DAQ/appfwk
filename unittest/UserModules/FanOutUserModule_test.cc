/**
 * @file FanOutUserModule class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#include "app-framework/UserModules/FanOutUserModule.hh"
#include "app-framework/Buffers/DequeBuffer.hh"
#include "app-framework-base/Services/CommandFacility.hh"

#define BOOST_TEST_MODULE FanOutUserModule_test

#include <boost/test/unit_test.hpp>

struct NonCopyableType {
    int data;
    NonCopyableType(int d) : data(d) {}
    NonCopyableType(NonCopyableType const&) = delete;
    NonCopyableType(NonCopyableType&& i) { data = i.data; }
    NonCopyableType& operator=(NonCopyableType const&) = delete;
    NonCopyableType& operator=(NonCopyableType&& i) {
        data = i.data;
        return *this;
    }
};

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ = nullptr;
}

BOOST_AUTO_TEST_SUITE(FanOutUserModule_test)

BOOST_AUTO_TEST_CASE(Construct) {
    auto buf = std::make_shared<appframework::DequeBuffer<int>>();
    appframework::FanOutUserModule<int> foum(buf, {buf});
}

BOOST_AUTO_TEST_CASE(Configure) {
    auto buf = std::make_shared<appframework::DequeBuffer<int>>();
    appframework::FanOutUserModule<int> foum(buf, {buf});
    foum.execute_command("configure");
}

BOOST_AUTO_TEST_CASE(NonCopyableTypeTest) {
    auto buf = std::make_shared<appframework::DequeBuffer<NonCopyableType>>();
    appframework::FanOutUserModule<NonCopyableType> foum(buf, {buf, buf});
    foum.execute_command("configure");
    foum.execute_command("start");

    NonCopyableType nct(1);
    buf->push(std::move(nct));

    foum.execute_command("stop");

    while (!buf->empty()) {
        auto out = buf->pop();
        if (out.data != 1) {
            throw std::runtime_error("Unexpected output from test!");
        }
    }

}

BOOST_AUTO_TEST_SUITE_END()
