#include<tuple>
#include<memory>
#include "TTask.h"
#include <vector>



class TTaskScheduler {
public:
    template<typename Task, typename... Args>
    auto Add(Task task, Args... args) {
        TTask new_task(task, args);
        tasks_.push_back(new_task);
        return new_task;
    }

    void ExecuteAll() {

    }
private:
    std::vector<TTask> tasks_;
};