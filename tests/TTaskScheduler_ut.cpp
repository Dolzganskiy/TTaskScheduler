#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../lib/TTaskScheduler.h"

static int global_counter = 0;
int IncrementCounter(int val) {
    global_counter += val;
    return global_counter;
}

class SchedulerTest : public ::testing::Test {
protected:
    void SetUp() override {
        global_counter = 0;
    }
};

TEST_F(SchedulerTest, Debug) {
    TTaskScheduler scheduler;

    auto t_debug = scheduler.Add([]() -> bool { return true; });
    static_assert(!std::is_void_v<decltype(t_debug.GetResultSync())>, "Task returns void!");
}

TEST_F(SchedulerTest, SimpleTaskExecution) {
    TTaskScheduler scheduler;
    auto task = scheduler.Add([](int a, int b) { return a + b; }, 10, 20);
    
    EXPECT_EQ(task.GetResultSync(), 30);
}

TEST_F(SchedulerTest, LazyEvaluation) {
    TTaskScheduler scheduler;
    auto task = scheduler.Add(IncrementCounter, 5);
    
    EXPECT_EQ(global_counter, 0);
    task.GetResultSync();
    EXPECT_EQ(global_counter, 5);
}

TEST_F(SchedulerTest, ChainingWithApply) {
    TTaskScheduler scheduler;
    auto task1 = scheduler.Add([]() { return 10; });
    auto task2 = task1.Apply([](int res) { return res * 2; });
    
    EXPECT_EQ(task2.GetResultSync(), 20);
}

TEST_F(SchedulerTest, DependencyViaFuture) {
    TTaskScheduler scheduler;
    
    auto task1 = scheduler.Add([]() { return std::string("Hello"); });
    auto task2 = scheduler.Add([](const std::string& s) { return s + " World"; }, 
                               task1.GetFutureResult<const std::string&>());
    
    scheduler.ExecuteAll();
    EXPECT_EQ(task2.GetResultSync(), "Hello World");
}

TEST_F(SchedulerTest, MoveSemanticsAndException) {
    TTaskScheduler scheduler;
    
    auto task = scheduler.Add([]() { return std::vector<int>{1, 2, 3}; });
    
    std::vector<int> result = task.GetResultSync();
    EXPECT_EQ(result.size(), 3);
    
    EXPECT_THROW(task.GetResultSync(), std::runtime_error);
}

TEST_F(SchedulerTest, ExecuteAllGraph) {
    TTaskScheduler scheduler;
    
    auto t1 = scheduler.Add([]() { return 1; });
    auto t2 = scheduler.Add([](int v) { return v + 1; }, t1.GetFutureResult<int>());
    auto t3 = scheduler.Add([](int v) { return v * 10; }, t2.GetFutureResult<int>());
    
    scheduler.ExecuteAll();
    
    EXPECT_EQ(t3.GetResultSync(), 20);
}

TEST_F(SchedulerTest, ApplyWithReferences) {
    TTaskScheduler scheduler;
    
    auto task = scheduler.Add([]() { return 100; });
    auto chained = task.Apply([](int& val) { 
        val += 50; 
        return val; 
    });
    
    EXPECT_EQ(chained.GetResultSync(), 150);
}