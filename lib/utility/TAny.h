#include <memory>
#include <type_traits>
#include <utility>

class Any {
public:
    Any() = default;

    template<typename T>
    Any(T&& v) : ptr_(std::make_unique<Holder<std::decay_t<T>>>(std::forward<T>(v))) {}

    bool HasValue() const { return ptr_ != nullptr; }

    template<typename T>
    T& Cast() {
        auto* derived = dynamic_cast<Holder<std::decay_t<T>>*>(ptr.get());
        if (!derived) throw std::runtime_error("Any: Wrong type cast!");
        return derived->value;
    }

    template<typename T>
    T MoveCast() {
        auto* derived = dynamic_cast<Holder<std::decay_t<T>>*>(ptr.get());
        if (!derived) throw std::runtime_error("Any: Wrong type cast!");
        return std::move(derived->value);
    }
private:
    struct Base {
        virtual ~Base() = default;
    };

    template<typename T>
    struct Holder : Base {
        T value_;
        Holder(T&& v) : value_(std::forward<T>(v)) {}
        Holder(const T& v) : value_(v) {}
    };
    
    std::unique_ptr<Base> ptr_;
};