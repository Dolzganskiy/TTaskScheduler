
struct TTaskBase {
    virtual ~TTaskBase() = 0;
};

template<typename... Args>
struct Node;

template<>
struct Node<> {};

template<typename T, typename... Args>
struct Node<T, Args...> {

};

template<typename Task, typename T, typename... Args> 
struct Node<Task, T, Args...> {

    Task task_;
    T value_;
    Node<Args..> tail_;
};



class TTask {
    
};