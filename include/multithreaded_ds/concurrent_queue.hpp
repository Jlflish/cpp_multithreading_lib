#pragma once

#include <mutex>

namespace multithreaded_ds {

template <typename T>
class concurrent_queue {
private:
    struct Node {
        Node *next;
        T data;
        Node(const T &init) : data(init), next(nullptr) {}
    };

    // front of the queue
    Node *front;
    // back of the queue
    Node *back;
    // size of the queue
    size_t queue_size;
    // lock of the stack
    std::mutex mtx;

public:
    concurrent_queue() noexcept : front(nullptr), back(nullptr), queue_size(0) {}
    
    ~concurrent_queue() noexcept {
        clear();
    }

    void push(const T& value) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        Node* new_node = new Node(value);
        if (front == nullptr) {
            front = back = new_node;
        } else {
            new_node->next = front;
            front = new_node;
        }
        queue_size++;
    }

    template <typename... Args>
    void emplace(Args&&... args) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        Node *new_node = new Node(std::forward<Args>(args)...);
        if (front == nullptr) {
            front = back = new_node;
        } else {
            new_node->next = front;
            front = new_node;
        }
        queue_size++;
    }

    bool pop(T& value) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        if (front == nullptr) {
            return false;
        }
        Node* old_front = front;
        value = old_front->data;
        front = old_front->next;
        delete old_front;
        queue_size--;
        return true;
    }

    bool isEmpty() noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        return front == nullptr;
    }

    size_t size() noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        return queue_size;
    }

    void clear() noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        while (front != nullptr) {
            Node* old_front = front;
            front = old_front->next;
            delete old_front;
        }
        queue_size = 0;
    }

    bool peek(T& value) noexcept {
        std::lock_guard<std::mutex> lock(mtx);
        if (front == nullptr) {
            return false;
        }
        value = front->data;
        return true;
    }
};

}  // namespace multithreaded_ds