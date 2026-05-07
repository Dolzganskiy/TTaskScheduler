#pragma once

#include <type_traits>
#include <memory>
#include <exception>
#include "utility/TAny.h"
#include "utility/NodeBase.h"

template<typename T>
class TFuture {
public:
    TFuture(std::shared_ptr<NodeBase> node) : node_(node) {}

    T Get() {
        node_->Execute();

        if (node_->WasMoved()) {
            throw std::runtime_error("Already moved");
        }

        auto& raw = node_->GetRawResult();

        if constexpr (std::is_reference_v<T>) {
            return raw.template Cast<std::remove_reference_t<T>>();
        } else {
            node_->MarkAsMoved();
            return raw.template MoveCast<T>();
        }
    }

    auto& Resolve() {
        return node_->GetRawResult().template Cast<T>();
    }
private:
    std::shared_ptr<NodeBase> node_;
};

template<typename T> 
struct is_future : std::false_type {};

template<typename T> 
struct is_future<TFuture<T>> : std::true_type {};

template<typename Arg>
decltype(auto) ResolveArg(Arg& arg) {
    if constexpr (is_future<std::decay_t<Arg>>::value) {
        return arg.Resolve();
    } else {
        return arg;
    }
}

template<typename T>
struct unwrap_future {
    using type = T;
};

template<typename T>
struct unwrap_future<TFuture<T>> {
    using type = T;
};

template<typename T>
using unwrap_future_t = typename unwrap_future<std::decay_t<T>>::type;