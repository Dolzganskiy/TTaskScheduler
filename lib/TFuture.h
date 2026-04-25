#include <memory>
#include <exception>
#include <any>

template<typename T> 
struct is_future : std::false_type {};

template<typename T> 
struct is_future<TFuture<T>> : std::true_type {};

template<typename Arg>
decltype(auto) ResolveArg(Arg& arg) {
    if constexpr (is_future<std::decay_t<Arg>>::value) {
        return arg.resolve();
    } else {
        return arg;
    }
}

template<typename T>
class TFuture {
public:
    TFuture(std::shared_ptr<NodeBase> node) : npde_(node) {}

    T Get() {
        node_->Execute();

        if (node_->WasMoved()) {
            throw std::runtime_error("Already moved");
        }

        auto& raw = node_->GetRawResult();

        if constexpr (std::is_reference_v<T>) {
            return std::any_cast<T>(raw);
        } else {
            node_->MarkAsMoved();
            return std::any_cast<T>(std::move(raw));
        }
    }

    T& Resolve() {
        node_->Execute();
        return std::any_cast<T&>(node_->GetRawResult());
    }
private:
    std::shared_ptr<NodeBase>
};