#include "../include/multithreaded_ds/concurrent_queue.hpp"
#include <iostream>
#include <thread>
#include <vector>

void test_single_thread() {
    multithreaded_ds::concurrent_queue<int> queue;
    
    // Test push and pop
    queue.push(42);
    int value;
    if (queue.pop(value)) {
        std::cout << "Popped value: " << value << std::endl;
    }
    
    // Test empty queue
    if (!queue.pop(value)) {
        std::cout << "Queue is empty as expected" << std::endl;
    }
    
    // Test peek
    queue.push(100);
    if (queue.peek(value)) {
        std::cout << "Peeked value: " << value << std::endl;
    }
}

void test_multi_thread() {
    multithreaded_ds::concurrent_queue<int> queue;
    std::vector<std::thread> threads;
    const int num_threads = 4;
    const int elements_per_thread = 1000;
    
    // Create producer threads that push values
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&queue, i]() {
            for (int j = 0; j < elements_per_thread; ++j) {
                queue.push(i * elements_per_thread + j);
            }
        });
    }
    
    // Wait for all producer threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify the size
    std::cout << "Queue size after pushes: " << queue.size() << std::endl;
    
    // Create consumer threads that pop values
    threads.clear();
    std::vector<int> consumed_values;
    std::mutex consumed_mutex;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&queue, &consumed_values, &consumed_mutex]() {
            int value;
            while (queue.pop(value)) {
                std::lock_guard<std::mutex> lock(consumed_mutex);
                consumed_values.push_back(value);
            }
        });
    }
    
    // Wait for all consumer threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Total values consumed: " << consumed_values.size() << std::endl;
    
    // Clear the queue
    queue.clear();
    std::cout << "Queue size after clear: " << queue.size() << std::endl;
}

int main() {
    std::cout << "Testing single thread operations..." << std::endl;
    test_single_thread();
    
    std::cout << "\nTesting multi-thread operations..." << std::endl;
    test_multi_thread();
    
    return 0;
}
