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
    auto inputbuf = std::make_shared<appframework::DequeBuffer<NonCopyableType>>();
    auto outputbuf1 = std::make_shared<appframework::DequeBuffer<NonCopyableType>>();
    auto outputbuf2 = std::make_shared<appframework::DequeBuffer<NonCopyableType>>();
    appframework::FanOutUserModule<NonCopyableType> foum(inputbuf, {outputbuf1, outputbuf2});
    
    // This test assumes RoundRobin mode. Once configurability is implemented, we'll have to configure it appropriately.
    foum.execute_command("configure");
    foum.execute_command("start");

    NonCopyableType nct1(1);
    inputbuf->push(std::move(nct1));
    NonCopyableType nct2(2);
    inputbuf->push(std::move(nct2));

    while (!inputbuf->empty())
    usleep(10000);

    foum.execute_command("stop");

    BOOST_REQUIRE_EQUAL(inputbuf->empty(), true);
    
    BOOST_REQUIRE_EQUAL(outputbuf1->empty(), false);
    auto res = outputbuf1->pop();
    BOOST_REQUIRE_EQUAL(res.data, 1);

    BOOST_REQUIRE_EQUAL(outputbuf2->empty(), false);
    res = outputbuf2->pop();
    BOOST_REQUIRE_EQUAL(res.data, 2);
}

BOOST_AUTO_TEST_SUITE_END()
