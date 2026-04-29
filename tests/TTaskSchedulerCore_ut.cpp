#include <gtest/gtest.h>
#include "../lib/TTaskScheduler.h"
#include <expected>

struct Data {
    int id;
    std::string val;
    bool operator==(const Data& other) const { return id == other.id && val == other.val; }
};

TEST(SchedulerCore, SimpleArithmetic) {
    TTaskScheduler scheduler;
    auto task1 = scheduler.Add([](int a, int b) { return a + b; }, 10, 20);
    scheduler.ExecuteAll();
    EXPECT_EQ(task1.GetResultSync(), 30);
}

TEST(SchedulerCore, ChainedDependencies) {
    TTaskScheduler scheduler;
    auto t1 = scheduler.Add([]() { return 10; });
    auto t2 = scheduler.Add([](int val) { return val * 2; }, t1.GetFutureResult<int>());
    
    scheduler.ExecuteAll();
    EXPECT_EQ(t2.GetResultSync(), 20);
}

TEST(SchedulerCore, MultipleInputs) {
    TTaskScheduler scheduler;
    auto t1 = scheduler.Add([]() { return std::string("Hello"); });
    auto t2 = scheduler.Add([]() { return std::string("World"); });
    auto t3 = scheduler.Add([](std::string a, std::string b) { 
        return a + " " + b; 
    }, t1.GetFutureResult<std::string>(), t2.GetFutureResult<std::string>());

    scheduler.ExecuteAll();
    EXPECT_EQ(t3.GetResultSync(), "Hello World");
}

TEST(SchedulerCore, VoidTaskHandling) {
    TTaskScheduler scheduler;
    bool flag = false;
    auto t1 = scheduler.Add([&]() { flag = true; });
    
    scheduler.ExecuteAll();
    EXPECT_TRUE(flag);
}

TEST(SchedulerCore, ApplyMethod) {
    TTaskScheduler scheduler;
    auto t1 = scheduler.Add([]() { return 5; });
    auto t2 = t1.Apply([](int& val) { return val + 5; });

    scheduler.ExecuteAll();
    EXPECT_EQ(t2.GetResultSync(), 10);
}

TEST(SchedulerCore, BasicTypesAndMath) {
    TTaskScheduler s;
    
    auto t1 = s.Add([]() { return 2 + 2; });
    auto t2 = s.Add([](int x) { return x * 10; }, t1.GetFutureResult<int>());
    auto t3 = s.Add([]() { return std::string("test"); });
    auto t4 = s.Add([](std::string s) { return s + "!"; }, t3.GetFutureResult<std::string>());
    auto t5 = s.Add([]() { return 0.5f; });
    auto t6 = s.Add([]() { return true; });
    auto t7 = s.Add([](bool b) { return !b; }, t6.GetFutureResult<bool>());
    auto t8 = s.Add([]() { return 'A'; });
    auto t9 = s.Add([](int x, float y) { return x + y; }, t1.GetFutureResult<int>(), t5.GetFutureResult<float>());
    auto t10 = s.Add([]() { return 100LL; });

    s.ExecuteAll();

    EXPECT_EQ(t1.GetResultSync(), 4);
    EXPECT_EQ(t2.GetResultSync(), 40);
    EXPECT_EQ(t4.GetResultSync(), "test!");
    EXPECT_EQ(t7.GetResultSync(), false);
    EXPECT_NEAR(t9.GetResultSync(), 4.5f, 0.001);
}


TEST(SchedulerCore, LinearChains) {
    TTaskScheduler s;
    
    auto c1 = s.Add([]() { return 1; });
    auto c2 = s.Add([](int x) { return x + 1; }, c1.GetFutureResult<int>());
    auto c3 = s.Add([](int x) { return x + 1; }, c2.GetFutureResult<int>());
    auto c4 = s.Add([](int x) { return x + 1; }, c3.GetFutureResult<int>());
    auto c5 = s.Add([](int x) { return x + 1; }, c4.GetFutureResult<int>());

    auto s1 = s.Add([]() { return std::string("A"); });
    auto s2 = s.Add([](std::string x) { return x + "B"; }, s1.GetFutureResult<std::string>());
    auto s3 = s.Add([](std::string x) { return x + "C"; }, s2.GetFutureResult<std::string>());
    auto s4 = s.Add([](std::string x) { return x + "D"; }, s3.GetFutureResult<std::string>());
    auto s5 = s.Add([](std::string x) { return x + "E"; }, s4.GetFutureResult<std::string>());

    s.ExecuteAll();

    EXPECT_EQ(c5.GetResultSync(), 5);
    EXPECT_EQ(s5.GetResultSync(), "ABCDE");
}

TEST(SchedulerCore, GraphTopologies) {
    TTaskScheduler s;
    
    auto root = s.Add([]() { return 10; });
    auto l1 = s.Add([](int x) { return x + 2; }, root.GetFutureResult<int>());
    auto r1 = s.Add([](int x) { return x * 3; }, root.GetFutureResult<int>());
    auto leaf = s.Add([](int x, int y) { return x + y; }, l1.GetFutureResult<int>(), r1.GetFutureResult<int>());

    auto k1 = s.Add([]() { return 5; });
    auto k2 = s.Add([]() { return 5; });
    auto mid = s.Add([](int a, int b) { return a + b; }, k1.GetFutureResult<int>(), k2.GetFutureResult<int>());
    auto out1 = s.Add([](int x) { return x * 2; }, mid.GetFutureResult<int>());
    auto out2 = s.Add([](int x) { return x - 2; }, mid.GetFutureResult<int>());

    s.ExecuteAll();

    EXPECT_EQ(leaf.GetResultSync(), 42);
    EXPECT_EQ(out1.GetResultSync(), 20);
    EXPECT_EQ(out2.GetResultSync(), 8);
}

TEST(SchedulerCore, ApplyAndReferences) {
    TTaskScheduler s;
    
    auto t1 = s.Add([]() { return 100; });
    
    auto a1 = t1.Apply([](int& x) { return x / 2; });
    auto a2 = a1.Apply([](int& x) { return x - 10; });
    auto a3 = a2.Apply([](int& x) { return std::to_string(x); });
    auto a4 = a3.Apply([](std::string& x) { return x.size(); });
    auto a5 = a4.Apply([](size_t& x) { return x == 2; });

    std::string secret = "secret";
    auto t_ref = s.Add([](const std::string& str) { return str.length(); }, secret);

    s.ExecuteAll();

    EXPECT_TRUE(a5.GetResultSync());
    EXPECT_EQ(t_ref.GetResultSync(), 6);
}

TEST(SchedulerCore, ContainersAndExpected) {
    TTaskScheduler s;
    
    auto v1 = s.Add([]() { return std::vector<int>{1, 2, 3}; });
    auto v2 = s.Add([](std::vector<int> v) { 
        v.push_back(4); 
        return v; 
    }, v1.GetFutureResult<std::vector<int>>());

    using OptInt = std::expected<int, std::string>;
    auto e1 = s.Add([]() -> OptInt { return 42; });
    auto e2 = s.Add([]() -> OptInt { return std::unexpected("error"); });
    
    auto check1 = s.Add([](OptInt res) { return res.has_value(); }, e1.GetFutureResult<OptInt>());
    auto check2 = s.Add([](OptInt res) { return res.error(); }, e2.GetFutureResult<OptInt>());

    auto d1 = s.Add([]() { return Data{1, "ok"}; });
    auto d2 = s.Add([](Data d) { return d.val == "ok"; }, d1.GetFutureResult<Data>());

    s.ExecuteAll();

    EXPECT_EQ(v2.GetResultSync().size(), 4);
    EXPECT_TRUE(check1.GetResultSync());
    EXPECT_EQ(check2.GetResultSync(), "error");
    EXPECT_TRUE(d2.GetResultSync());
}

int AddFunc(int a, int b) { return a + b; }
TEST(SchedulerFullCoverage, AddPlainFunction) {
    TTaskScheduler s;
    auto t = s.Add(AddFunc, 5, 5);
    s.ExecuteAll();
    EXPECT_EQ(t.GetResultSync(), 10);
}

TEST(SchedulerFullCoverage, AddLambdaCapture) {
    TTaskScheduler s;
    int multiplier = 3;
    auto t = s.Add([&](int x) { return x * multiplier; }, 10);
    s.ExecuteAll();
    EXPECT_EQ(t.GetResultSync(), 30);
}

TEST(SchedulerFullCoverage, DependencyChain) {
    TTaskScheduler s;
    auto t1 = s.Add([]() { return std::string("Hello"); });
    auto t2 = s.Add([](std::string msg) { return msg + " World"; }, t1.GetFutureResult<std::string>());
    s.ExecuteAll();
    EXPECT_EQ(t2.GetResultSync(), "Hello World");
}

TEST(SchedulerFullCoverage, ApplyTypeTransformation) {
    TTaskScheduler s;
    auto t1 = s.Add([]() { return 100; });
    auto t2 = t1.Apply([](int& val) { return std::to_string(val); });
    s.ExecuteAll();
    EXPECT_EQ(t2.GetResultSync(), "100");
}

TEST(SchedulerFullCoverage, MultipleDependencies) {
    TTaskScheduler s;
    auto t1 = s.Add([]() { return 10; });
    auto t2 = s.Add([]() { return 20; });
    auto t3 = s.Add([](int a, int b) { return a + b; }, t1.GetFutureResult<int>(), t2.GetFutureResult<int>());
    s.ExecuteAll();
    EXPECT_EQ(t3.GetResultSync(), 30);
}