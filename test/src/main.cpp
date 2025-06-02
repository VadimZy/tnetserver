//
// Created by vadimzy on 6/1/25.
//

#include <iostream>
#include <gtest/gtest.h>

#include <bits/ostream.tcc>

namespace testing {
    class TestPartResult;
    class TestInfo;
}

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
    std::cout << "Hello Test World!" << std::endl;
    testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new EventListener());

    ::testing::AddGlobalTestEnvironment(new UtEnvironment());
    const int exit_status = RUN_ALL_TESTS();

    //google::protobuf::ShutdownProtobufLibrary();

    return exit_status;
    return 0;
}
