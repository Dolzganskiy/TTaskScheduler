#pragma once

#include <utility>
#include <type_traits>

template<typename F, typename... Args>
decltype(auto) Invoke(F&& f, Args&&... args) {
    if constexpr (std::is_member_function_pointer_v<std::decay_t<F>>) {
        auto helper = [](auto&& obj, auto&& method, auto&&... params) -> decltype(auto) {
            if constexpr (std::is_pointer_v<std::decay_t<decltype(obj)>>) {
                return (obj->*method)(std::forward<decltype(params)>(params)...);
            } else {
                return (obj.*method)(std::forward<decltype(params)>(params)...);
            }
        };
        return helper(std::forward<Args>(args)..., std::forward<F>(f));
    } else {
        return std::forward<F>(f)(std::forward<Args>(args)...);
    }
}