
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
            return std::any_cast
        }
    }
private:
    std::shared_ptr<NodeBase>
};