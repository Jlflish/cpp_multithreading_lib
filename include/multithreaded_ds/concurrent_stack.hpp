#ifndef CONCURRENT_STACK_HPP
#define CONCURRENT_STACK_HPP

#include <atomic>
#include <memory>

namespace multithreaded_ds {

template <typename T>
class concurrent_stack {
public:
    concurrent_stack();

    ~concurrent_stack();

    void push(const T& value);

    bool pop(T& value);

    bool isEmpty() const;

    size_t size() const;

    void clear();

    bool peek(T& value) const;

private:
    struct Node {
        T data;
        std::atomic<Node*> next;

        Node(const T& value) : data(value), next(nullptr) {}
    };

    // top pointer of the stack
    std::atomic<Node*> top;
    // size of the stack
    std::atomic<size_t> stack_size;
};

}  // namespace multithreaded_ds

#endif
