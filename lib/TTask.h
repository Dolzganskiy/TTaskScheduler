#include<memory>
#include<utility>
#include "TFuture.h"
#include<cstddef>
#include<string>
#include<type_traits>
#include<optional>
#include <stdexcept>
#include <typeinfo>

struct NodeBase {
    virtual ~NodeBase() = default;
    virtual void Execute() = 0;
    virtual void* GetRawResult() = 0;
    virtual const std::type_info& GetResultType() const = 0;
};

template<typename... Args>
struct Node;

template<>
struct Node<> {
    static constexpr std::size_t arg_count = 0;
};

template<typename T, typename... Args>
struct Node<T, Args...> {
    static constexpr std::size_t arg_count = 1 + Node<Args...>::arg_count;

    Node(T&& arg, Args&&... args) 
        : arg_(std::forward<T>(arg)), 
          tail_(std::forward<Args>(args)...) 
    {}

    T arg_;
    Node<Args...> tail_;
};

template<std::size_t N, typename NodeType>
struct NodeGet;

template<std::size_t N, typename T, typename... Args>
struct NodeGet<N, Node<T, Args...>> {
    static auto& Get(Node<T, Args...>& node) {
        return NodeGet<N - 1, Node<Args...>>::Get(node.tail_);
    }
};

template<typename T, typename... Args>
struct NodeGet<0, Node<T, Args...>> {
    static T& Get(Node<T, Args...>& node) {
        return node.arg_;
    }
};

template<typename Task, typename... Args> 
struct TaskNode : NodeBase {

    using result_type = std::invoke_result_t<Task, Args...>;

    TaskNode& operator=(const TaskNode& other) {
        task
    }

    TaskNode(Task&& task, Args&&... args) 
        : task_(std::forward<Task>(task)), 
          args_(std::forward<Args>(args)...) 
    {}

    void Execute() override {

        if (executed_) {
            return;
        }

        if constexpr (std::is_void_v<result_type>) {
            ExecuteImpl(std::make_index_sequence<sizeof...(Args)>{});
        } else {
            result_ = ExecuteImpl(std::make_index_sequence<sizeof...(Args)>{});
        }

        executed_ = true;
    }

    void* GetRawResult() override {
        if constexpr (std::is_void_v<result_type>) {
            return nullptr;
        } else {
            return result_ ? &(*result_) : nullptr;
        }
    } 

    const std::type_info& GetResultType() const override {
        return typeid(result_type);
    }

    void MakeNewTask()

private:
    Task task_;
    Node<Args...> args_;

    bool executed_ = false;
    std::optional<result_type> result_;

    template<std::size_t... Ind>
    result_type ExecuteImpl(std::index_sequence<Ind...>) {
        return task_(NodeGet<Ind, Node<Args...>>::Get(args_)...);
    }

};

template<typename Task>
void UpdateTask(Task task) {

}

class TTask {
public:

    template<typename Task, typename... Args>
    TTask(Task&& task, Args&&... args) {
        node_ = std::make_unique<TaskNode<Task, Args...>>(std::forward<Task>(task), std::forward<Args>(args)...);

    }

    template<typename T>
    T GetResultSync() {

        static_assert(!std::is_void_v<T>, "Cannot get result of void task");

        if (!node_) {
            throw std::runtime_error("No Task Stored");
        }

        node_->Execute();

        if (node_->GetResultType() != typeid(T)) {
            throw std::runtime_error("Wrong result type requested");
        }

        void* raw = node_->GetRawResult();

        if (!raw) {
            throw std::runtime_error("Task has void result");
        }
    
        return std::move(*static_cast<T*>(raw));
    }

    template<typename T>
    TFuture<T> GetFutureResult() {
        return TFuture<T>{};
    }

    template<typename Task, typename... Args>
    TTask apply(Task task) {

    }

    void Execute() {
        if (node_) {
            node_->Execute();
        }
    }

private:
    std::unique_ptr<NodeBase> node_;

};