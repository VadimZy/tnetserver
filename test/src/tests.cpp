//
// Created by vadimzy on 6/1/25.
//
#include <thread>
#include <gtest/gtest.h>

#include "../../util/util.h"
#include "../../src/DigestGenerators.h"
#include "../../src/TcpServer.h"
#include "../../src/HashEchoClient.h"
#include "../../util/logger.h"

COMMON_LOGGER();

// input splitter test
TEST(buff_split, splitter) {
    char buf[100] = "11111111111111\n|11111111111222222|";

    auto sp = util::BuffSplitter({buf, strlen(buf)});
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left() == "11111111111111");
    ASSERT_TRUE(sp.right() == "|11111111111222222|");

    char buf1[100] = "qqqqqqqqqqqqqqq";
    sp = util::BuffSplitter({buf1, strlen(buf1)});
    ASSERT_FALSE(sp.has_split());
    ASSERT_TRUE(sp.left() == "qqqqqqqqqqqqqqq");
    ASSERT_TRUE(sp.right().empty());

    char buf2[100] = "qqqqqqqqqqqqqqq\n";
    sp = util::BuffSplitter({buf2, strlen(buf2)});
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left() == "qqqqqqqqqqqqqqq");
    ASSERT_TRUE(sp.right().empty());

    char buf3[100] = "\nqqqqqqqqqqqqqqq";
    sp = util::BuffSplitter({buf3, strlen(buf3)});
    ASSERT_TRUE(sp.has_split());
    ASSERT_TRUE(sp.left().empty());
    ASSERT_TRUE(sp.right()== "qqqqqqqqqqqqqqq");
}

// input stream hash test
TEST(buff_split, hash_stream) {
    MD5Digest md5;
    StreamMD5Digest sd('\n');

    std::vector<std::string> result;

    auto pFun = [&](std::string s) {
        result.emplace_back(s);
        return 0;
    };

    sd.append(pFun, "11111111111111\n|11111111111222222|");
    ASSERT_TRUE(result.size() == 1);

    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[0]);

    sd.reset();
    result.clear();
    sd.append(pFun, "11111111111111\n|11111111111222222|\n");
    ASSERT_TRUE(result.size() == 2);
    md5.reset();
    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[0]);

    md5.reset();
    md5.update("|11111111111222222|");
    ASSERT_EQ(md5.to_hex_string(), result[1]);

    // append one \n
    sd.reset();
    result.clear();
    sd.append(pFun, "11111111111111\n|11111111111222222|");
    sd.append(pFun, "\n");
    ASSERT_TRUE(result.size() == 2);
    md5.reset();
    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[0]);

    md5.reset();
    md5.update("|11111111111222222|");
    ASSERT_EQ(md5.to_hex_string(), result[1]);


    // \n is the first char
    sd.reset();
    result.clear();
    sd.append(pFun, "\n11111111111111\n|11111111111222222|\n");
    ASSERT_TRUE(result.size() == 3);
    md5.reset();
    md5.update("11111111111111");
    ASSERT_EQ(md5.to_hex_string(), result[1]);

    md5.reset();
    md5.update("|11111111111222222|");
    ASSERT_EQ(md5.to_hex_string(), result[2]);
}

// test server helper
class TestTcpServer {
public:
    bool externServer{false};
    std::string ncOption;// = " -N ";

    explicit TestTcpServer(int p): port(p) {

        //check if run with external server
        externServer = getenv("USE_EXTERNAL_TNETSERVER") != nullptr;

        // check nc version
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("echo test | nc  -N localhost 2>&1", "r"), pclose);

        std::string result{};
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result += buffer;
        }

        if (result.find("invalid option") != std::string::npos) {
            ncOption.clear();
        }

    }

    void start() {
        if (externServer) {
            return;
        }

        spid = fork();
        if (spid == 0) {
            serverPtr = std::make_shared<TcpServer>(2323, std::make_unique<HashEchoClientFactory>());
            serverThreadPtr = std::make_unique<std::thread>([&]() { serverPtr->run(); });
        } else {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(3s);
        }
    }

    void stop() {
        if (spid > 0) {
            kill(spid, SIGTERM);
            spid = -1;
        }
        if (serverThreadPtr) {
            serverThreadPtr->join();
        }
    }

    [[nodiscard]] std::string sendToServerAsync(const std::string &str, int cnt = 1) const {
        std::string result;
        auto t = std::thread([&]() {
            result = sendToServer(str, cnt);
        });
        t.join();
        return result;
    }

    [[nodiscard]] std::string sendToServer(const std::string &str, int cnt = 1) const {
        std::stringstream ss;
        ss << "echo " << str << " | nc " << ncOption << " localhost " << port;
        for (int i = 1; i < cnt; ++i) {
            ss << "& echo " << str << " | nc " << ncOption << " localhost " << port;
        }

        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(ss.str().c_str(), "r"), pclose);
        if (!pipe) {
            LOG_ERROR("broken pipe");
            return {};
        }

        std::string result{};
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result += buffer;
        }

        return result;
    }

    ~TestTcpServer() {
        stop();
    }

private:
    int port{};
    int spid{-1};
    std::shared_ptr<TcpServer> serverPtr;
    std::unique_ptr<std::thread> serverThreadPtr;
};

// hex helper
auto to_hex(const std::string &s, int cnt = 1) {
    MD5Digest md5;
    md5.update(s);
    auto h = md5.to_hex_string();
    auto ret = h + '\n';

    for (int i = 1; i < cnt; ++i) {
        ret += h;
        ret += '\n';
    }
    return ret;
}

// smoke test
TEST(server, simple) {
    TestTcpServer srv(2323);
    srv.start();
    using namespace std::chrono_literals;

    std::string tStr = "qqqqqqq";
    auto hex = to_hex(tStr);

    bool good = true;

    for (int i = 0; i < 100; i++) {
        //LOG_INFO("\n  === task: %d === \n", i);
        auto res = srv.sendToServer(tStr);
        good = hex == res;
        if (!good) {
            LOG_ERROR("md5 failed: i=%d result: %s, expected: %s", i, res.c_str(), hex.c_str());
            break;
        }
    }
    ASSERT_TRUE(good);
    srv.stop();
}

// 100 concurrent short strings, 3 connections
TEST(server, fuzz) {
    TestTcpServer srv(2323);
    srv.start();
    using namespace std::chrono_literals;
    std::string tStr{8, 'q'};
    auto hex = to_hex(tStr, 3);

    std::vector<std::string> results;
    std::mutex mtx;

    std::vector<std::thread> cmdThreads;

    for (int i = 0; i < 100; i++) {
        cmdThreads.emplace_back([&]() {
            auto lk = std::lock_guard(mtx);
            results.emplace_back(srv.sendToServer(tStr, 3));
        });
    }

    for (auto &t: cmdThreads) {
        t.join();
    }

    auto good{false};

    for (const auto &r: results) {
        good = hex == r;
        if (!good) {
            LOG_ERROR("md5 failed: result: %s, expected: %s", r.c_str(), hex.c_str());
            break;
        }
    }
    ASSERT_TRUE(good);

    srv.stop();
}

// 100 concurrent long strings, 10 connections
TEST(server, fuzz1) {
    TestTcpServer srv(2323);
    srv.start();
    using namespace std::chrono_literals;
    std::string tStr(4000, 'q');
    auto hex = to_hex(tStr, 10);

    std::vector<std::string> results;
    std::mutex mtx;

    std::vector<std::thread> cmdThreads;

    for (int i = 0; i < 100; i++) {
        cmdThreads.emplace_back([&]() {
            auto lk = std::lock_guard(mtx);
            results.emplace_back(srv.sendToServer(tStr, 10));
        });
    }

    for (auto &t: cmdThreads) {
        t.join();
    }

    auto good{false};

    for (const auto &r: results) {
        good = hex == r;
        if (!good) {
            LOG_ERROR("md5 failed: result: %s, expected: %s", r.c_str(), hex.c_str());
            break;
        }
    }
    ASSERT_TRUE(good);
}
