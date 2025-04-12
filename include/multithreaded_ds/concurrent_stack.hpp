#ifndef CONCURRENT_STACK_HPP
#define CONCURRENT_STACK_HPP

#include <atomic>
#include <memory>
#include <mutex>
#include <cstddef>

namespace multithreaded_ds {

template <typename T>
class concurrent_stack {
private:
    struct Node {
        Node *next;
        T data;
        Node(const T &init) : data(init), next(nullptr) {}
    };

    // top of the stack
    Node *top;
    // size of the stack
    size_t stack_size;
    // lock of the stack
    std::mutex mtx;

public:
    concurrent_stack() noexcept : top(nullptr), stack_size(0) {}
    
    ~concurrent_stack() noexcept {
        clear();
    }

    void push(const T& value) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        Node* new_node = new Node(value);
        new_node->next = top;
        top = new_node;
        stack_size++;
    }

    bool pop(T& value) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        if (top == nullptr) return false;
        
        Node* old_top = top;
        value = old_top->data;
        top = old_top->next;
        delete old_top;
        stack_size--;
        return true;
    }

    bool isEmpty() noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        return top == nullptr;
    }

    size_t size() noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        return stack_size;
    }

    void clear() noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        while (top != nullptr) {
            Node* old_top = top;
            top = old_top->next;
            delete old_top;
        }
        stack_size = 0;
    }

    bool peek(T& value) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        if (top == nullptr) return false;
        value = top->data;
        return true;
    }
};

}  // namespace multithreaded_ds
#endif