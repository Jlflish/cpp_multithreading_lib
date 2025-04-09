#ifndef CONCURRENT_STACK_HPP
#define CONCURRENT_STACK_HPP

#include <atomic>
#include <memory>

namespace multithreaded_ds {

template <typename T>
class concurrent_stack {
public:
    concurrent_stack() : top(nullptr), stack_size(0) {}

    ~concurrent_stack() {
        clear();
    }

    void push(const T& value) {
        Node* new_node = new Node(value);
        Node* expected = top.load();
        new_node->next = expected;
        while (!top.compare_exchange_weak(expected, new_node)) {
            new_node->next = expected;
        }
        stack_size.fetch_add(1);
    }

    bool pop(T& value) {
        Node* old_top = top.load();
        if (old_top == nullptr) return false;

        while (!top.compare_exchange_weak(old_top, old_top->next.load())) {
            if (old_top == nullptr) return false;
        }

        value = old_top->data;
        delete old_top;
        stack_size.fetch_sub(1);
        return true;
    }

    bool isEmpty() const {
        return top.load() == nullptr;
    }

    size_t size() const {
        return stack_size.load();
    }

    void clear() {
        T value;
        while (pop(value)) {
            // Keep popping until empty
        }
    }

    bool peek(T& value) const {
        Node* current_top = top.load();
        if (current_top == nullptr) return false;
        value = current_top->data;
        return true;
    }

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
