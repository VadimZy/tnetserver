//
// Created by vadimzy on 6/1/25.
//

#include <iostream>
#include <gtest/gtest.h>

#include <bits/ostream.tcc>
#include "../../util/logger.h"

namespace testing {
    class TestPartResult;
    class TestInfo;
}

COMMON_LOGGER();

class EventListener : public ::testing::EmptyTestEventListener {
public:
    EventListener() = default;

    // Called before a test starts.
    void OnTestStart(const ::testing::TestInfo &test_info) override {
    }

    // Called after a failed assertion or a SUCCEED() invocation.
    void OnTestPartResult(const ::testing::TestPartResult &test_part_result) override {
    }

    // Called after a test ends.
    void OnTestEnd(const ::testing::TestInfo &test_info) override {
    }

private:
    bool keepFiles{};
};

class UtEnvironment : public ::testing::Environment {
public:
    UtEnvironment() = default;
};

int main(int argc, char* argv[]) {

    // setup logger
    util::log::log_sink::use_console_log();
    util::log::log_sink::set_level("debug");

    LOG_INFO("Starting testing");

    testing::InitGoogleTest(&argc, argv);

    // test helpers
    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new EventListener());
    ::testing::AddGlobalTestEnvironment(new UtEnvironment());

    // run tests
    return RUN_ALL_TESTS();
}
