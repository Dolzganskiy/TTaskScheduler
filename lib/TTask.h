#include<memory>
#include<utility>
#include "TFuture.h"
#include<cstddef>
#include<string>
#include<type_traits>
#include<optional>
#include <stdexcept>
#include <typeinfo>
#include <any>

struct NodeBase {
    virtual ~NodeBase() = default;
    virtual void Execute() = 0;
    virtual std::any& GetRawResult() = 0;
    virtual bool WasMoved() const = 0;
    virtual void MarkAsMoved() = 0;
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

    TaskNode(Task&& task, Args&&... args) 
        : task_(std::forward<Task>(task)), 
          args_(std::forward<Args>(args)...) 
    {}

    void Execute() override {
        if (executed_) return;

        result_ = ExecuteImpl(std::make_index_sequence<sizeof...(Args)>{});
        executed_ = true;
    }

    std::any& GetRawResult() override { return result_; } 
    bool WasMoved() const override { return moved_; }
    void MarkAsMoved() override { moved_ = true; }

private:
    Task task_;
    Node<Args...> args_;

    bool executed_ = false;
    bool moved_ = false;
    std::any result_;

    template<std::size_t... Ind>
    result_type ExecuteImpl(std::index_sequence<Ind...>) {
        return task_(NodeGet<Ind, Node<Args...>>::Get(args_)...);
    }
};

template<typename PrevType, typename NewTask>
struct ChainNode : NodeBase {
public:
    using result_type = std::invoke_result_t<NewTask, PrevType>;

    ChainNode(std::shared_ptr<NodeBase> prev, NewTask&& task) 
        : prev_(prev), task_(std::forward<NewTask>(task))
    {}

    void Execute() override {
        if (executed_) return;

        prev_->Execute();
        PrevType prev_res = std::any_cast<PrevType>(prev_->GetRawResult());

        result_ = std::invoke(task_; std::forward<PrevType>(prev_res));
        executed_ = true;
    }

    void* GetRawResult() override { return &result_; }
    bool WasMoved() const override { return moved_; }
    void MarkAsMoved() override { moved_ = true; }

private:
    std::shared_ptr<NodeBase> prev_;
    NewTask task_;
    bool executed_ = false;
    bool moved_ = false;
    std::any result_;
};


class TTask {
public:

    TTask() = default;

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

        std::any* raw = static_cast<std::any*>(node_->GetRawResult());

        if (!raw) {
            throw std::runtime_error("Task returned void");
        }

        if (raw->type() != typeid(T)) {
            throw std::runtime_error("Wrong result type");
        }
    
        return std::any_cast<T>(*raw);
    }

    template<typename T>
    TFuture<T> GetFutureResult() {
        return TFuture<T>{};
    }

    template<typename NewTask>
    TTask Apply(NewTask&& task) {

        if (!node_) {
            throw std::runtime_error("No task to apply to");
        }

        TTask new_t;

        new_t.node_ = std::make_unique<ChainNode<NewTask>>(
                    std::move(node_), std::forward<NewTask>(task)
        );

        return new_t;
    }

    void Execute() {
        if (node_) {
            node_->Execute();
        }
    }

private:
    std::unique_ptr<NodeBase> node_;

};