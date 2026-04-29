#pragma once

#include<memory>
#include<utility>
#include<type_traits>
#include <stdexcept>
#include "utility/TAny.h"
#include "utility/TInvoke.h"
#include "TFuture.h"
#include "utility/NodeBase.h"

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

    TaskNode(Task&& task, Args&&... args) 
        : task_(std::forward<Task>(task)), 
          args_(std::forward<Args>(args)...) 
    {}

    void Execute() override {
        if (executed_) return;

        using ReturnType = std::invoke_result_t<Task, unwrap_future_t<Args>...>;

        if constexpr (std::is_void_v<ReturnType>) {
            ExecuteImpl(std::make_index_sequence<sizeof...(Args)>{});
        } else {
            result_ = ExecuteImpl(std::make_index_sequence<sizeof...(Args)>{});
        }
        executed_ = true;
    }

    Any& GetRawResult() override { return result_; } 
    bool WasMoved() const override { return moved_; }
    void MarkAsMoved() override { moved_ = true; }

private:
    Task task_;
    Node<Args...> args_;

    bool executed_ = false;
    bool moved_ = false;
    Any result_;

    template<std::size_t... Ind>
    auto ExecuteImpl(std::index_sequence<Ind...>) {
        return Invoke(task_, ResolveArg(NodeGet<Ind, Node<Args...>>::Get(args_))...);
    }
};

template<typename PrevType, typename NewTask>
struct ChainNode : NodeBase {
public:
    ChainNode(std::shared_ptr<NodeBase> prev, NewTask&& task) 
        : prev_(prev), task_(std::forward<NewTask>(task))
    {}

    void Execute() override {
        if (executed_) return;

        prev_->Execute();

        result_ = Invoke(task_, prev_->GetRawResult().template Cast<std::decay_t<PrevType>>());
        executed_ = true;
    }

    Any& GetRawResult() override { return result_; }
    bool WasMoved() const override { return moved_; }
    void MarkAsMoved() override { moved_ = true; }

private:
    std::shared_ptr<NodeBase> prev_;
    NewTask task_;
    bool executed_ = false;
    bool moved_ = false;
    Any result_;
};

template<typename U>
class TTask {
public:
    TTask(std::shared_ptr<NodeBase> node) : node_(node) {}

    U GetResultSync() {
        return TFuture<U>(node_).Get();
    }

    template<typename T = U>
    TFuture<T> GetFutureResult() {
        return TFuture<T>(node_);
    }

    template<typename NewTask>
    auto Apply(NewTask&& task) {
        using NextResult = std::invoke_result_t<NewTask, U&>;
        
        auto new_node = std::make_shared<ChainNode<U, NewTask>>(
            node_, std::forward<NewTask>(task)
        );
        return TTask<NextResult>(new_node);
        
    }

private:
    std::shared_ptr<NodeBase> node_;
};