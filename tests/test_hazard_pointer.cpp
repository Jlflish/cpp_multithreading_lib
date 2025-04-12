#include "../include/multithreaded_ds/hazard_pointer.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include <chrono>

struct Node {
    std::atomic<Node*> next;
    int val;
    Node(int init = 0) : next(nullptr), val(init) {}
};

class LockFreeList {
private:
    std::atomic<Node*> head;
    multithreaded_ds::hazard_pointer hp;

public:
    LockFreeList() : head(nullptr) {}

    void insert(int val) {
        Node* new_node = new Node(val);
        Node* old_head = head.load(std::memory_order_relaxed);
        do {
            new_node->next.store(old_head, std::memory_order_relaxed);
        } while (!head.compare_exchange_weak(old_head, new_node, 
                std::memory_order_release, std::memory_order_relaxed));
    }

    bool remove(int val) {
        Node* prev = nullptr;
        Node* curr = nullptr;
        Node* next = nullptr;

        while (true) {
            prev = head.load(std::memory_order_acquire);
            if (!hp.protect(reinterpret_cast<const std::atomic<void*>&>(head))) {
                continue;
            }
            curr = prev;
            
            while (curr != nullptr) {
                if (curr->val == val) {
                    if (prev == curr) {
                        if (head.compare_exchange_strong(prev, curr->next,
                                std::memory_order_release, std::memory_order_relaxed)) {
                            hp.retire(curr, [](void* ptr) { delete static_cast<Node*>(ptr); });
                            return true;
                        }
                    } else {
                        if (prev->next.compare_exchange_strong(curr, curr->next,
                                std::memory_order_release, std::memory_order_relaxed)) {
                            hp.retire(curr, [](void* ptr) { delete static_cast<Node*>(ptr); });
                            return true;
                        }
                    }
                }
                prev = curr;
                curr = curr->next;
            }
            return false;
        }
    }

    bool contains(int val) {
        Node* curr = head.load(std::memory_order_acquire);
        if (!hp.protect(reinterpret_cast<const std::atomic<void*>&>(head))) {
            return false;
        }
        
        while (curr != nullptr) {
            if (curr->val == val) {
                return true;
            }
            curr = curr->next;
        }
        return false;
    }
};

void worker(LockFreeList& list, int thread_id, int num_operations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> op_dist(0, 2);
    std::uniform_int_distribution<> val_dist(0, 100);

    for (int i = 0; i < num_operations; ++i) {
        int val = val_dist(gen);
        int op = op_dist(gen);

        switch (op) {
            case 0:
                list.insert(val);
                break;
            case 1:
                list.remove(val);
                break;
            case 2:
                list.contains(val);
                break;
        }
    }
}

int main() {
    const int num_threads = 4;
    const int num_operations = 10000;
    LockFreeList list;
    std::vector<std::thread> thread_pool;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        thread_pool.emplace_back(worker, std::ref(list), i, num_operations);
    }

    for (auto& t : thread_pool) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Test completed in " << duration.count() << " ms" << std::endl;
    std::cout << "All threads finished successfully" << std::endl;

    return 0;
}