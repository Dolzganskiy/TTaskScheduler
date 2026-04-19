#include<tuple>
#include<memory>
#include "TTask.h"
#include <vector>



class TTaskScheduler {
public:
    template<typename Task, typename... Args>
    auto Add(Task task, Args... args) {
        std::unique_ptr<BaseTTask> = new
    }

    void ExecuteAll() {

    }
private:
    std::vector<std::unique_ptr<TTask>> tasks_;
};