#pragma once

#include<tuple>
#include<memory>
#include "TTask.h"
#include <vector>

class TTaskScheduler {
public:
    template<typename Task, typename... Args>
    auto Add(Task&& task, Args&&... args) {
        using ResultType = std::invoke_result_t<Task, unwrap_future_t<Args>...>;
        auto node = std::make_shared<TaskNode<Task, Args...>>(std::forward<Task>(task), std::forward<Args>(args)...);
        tasks_.push_back(node);
        return TTask<ResultType>(node);
    }

    void ExecuteAll() {
        for (auto& node : tasks_) {
            node->Execute();
        }
    }

private:
    std::vector<std::shared_ptr<NodeBase>> tasks_;
};