#include "../include/multithreaded_ds/concurrent_stack.hpp"
#include <iostream>
#include <thread>
#include <vector>

void test_single_thread() {
    multithreaded_ds::concurrent_stack<int> stack;
    
    // Test push and pop
    stack.push(42);
    int value;
    if (stack.pop(value)) {
        std::cout << "Popped value: " << value << std::endl;
    }
    
    // Test empty stack
    if (!stack.pop(value)) {
        std::cout << "Stack is empty as expected" << std::endl;
    }
}

void test_multi_thread() {
    multithreaded_ds::concurrent_stack<int> stack;
    std::vector<std::thread> threads;
    
    // Create 4 threads that push values
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&stack, i]() {
            for (int j = 0; j < 1000; ++j) {
                stack.push(i * 1000 + j);
            }
        });
    }
    
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify the size
    std::cout << "Stack size after pushes: " << stack.size() << std::endl;
    
    // Clear the stack
    stack.clear();
    std::cout << "Stack size after clear: " << stack.size() << std::endl;
}

int main() {
    std::cout << "Testing single thread operations..." << std::endl;
    test_single_thread();
    
    std::cout << "\nTesting multi-thread operations..." << std::endl;
    test_multi_thread();
    
    return 0;
}