#include "../include/multithreaded_ds/concurrent_queue.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>

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

void test_concurrent_producer_consumer() {
    multithreaded_ds::concurrent_queue<int> queue;
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    const int num_producers = 3;
    const int num_consumers = 3;
    const int elements_per_producer = 1000;
    std::atomic<int> total_produced{0};
    std::atomic<int> total_consumed{0};
    std::atomic<bool> stop_flag{false};
    
    // Create producer threads
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&queue, i, &total_produced]() {
            for (int j = 0; j < elements_per_producer; ++j) {
                queue.push(i * elements_per_producer + j);
                total_produced++;
                // Add some random delay to increase contention
                std::this_thread::sleep_for(std::chrono::microseconds(rand() % 10));
            }
        });
    }
    
    // Create consumer threads
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, &total_consumed, &stop_flag]() {
            int value;
            while (!stop_flag || !queue.isEmpty()) {
                if (queue.pop(value)) {
                    total_consumed++;
                    // Add some random delay to increase contention
                    std::this_thread::sleep_for(std::chrono::microseconds(rand() % 10));
                }
            }
        });
    }
    
    // Wait for all producers to finish
    for (auto& thread : producers) {
        thread.join();
    }
    
    // Signal consumers to stop
    stop_flag = true;
    
    // Wait for all consumers to finish
    for (auto& thread : consumers) {
        thread.join();
    }
    
    std::cout << "Concurrent producer-consumer test:" << std::endl;
    std::cout << "Total values produced: " << total_produced << std::endl;
    std::cout << "Total values consumed: " << total_consumed << std::endl;
    std::cout << "Final queue size: " << queue.size() << std::endl;
}

void test_rapid_concurrent_operations() {
    multithreaded_ds::concurrent_queue<int> queue;
    const int num_threads = 8;
    const int operations_per_thread = 10000;
    std::vector<std::thread> threads;
    std::atomic<int> successful_pops{0};
    std::atomic<int> successful_pushes{0};
    
    // Create threads that perform rapid push and pop operations
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&queue, &successful_pops, &successful_pushes]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 1);
            
            for (int j = 0; j < operations_per_thread; ++j) {
                if (dis(gen) == 0) {
                    // Push operation
                    queue.push(j);
                    successful_pushes++;
                } else {
                    // Pop operation
                    int value;
                    if (queue.pop(value)) {
                        successful_pops++;
                    }
                }
            }
        });
    }
    
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "\nRapid concurrent operations test:" << std::endl;
    std::cout << "Successful pushes: " << successful_pushes << std::endl;
    std::cout << "Successful pops: " << successful_pops << std::endl;
    std::cout << "Final queue size: " << queue.size() << std::endl;
}

int main() {
    std::cout << "Testing single thread operations..." << std::endl;
    test_single_thread();
    
    std::cout << "\nTesting multi-thread operations..." << std::endl;
    test_multi_thread();
    
    std::cout << "\nTesting concurrent producer-consumer scenario..." << std::endl;
    test_concurrent_producer_consumer();
    
    std::cout << "\nTesting rapid concurrent operations..." << std::endl;
    test_rapid_concurrent_operations();
    
    return 0;
}
