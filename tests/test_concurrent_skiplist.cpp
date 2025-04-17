#include "../include/multithreaded_ds/concurrent_skiplist.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>
#include <algorithm>
#include <stdexcept>
#include <cassert>

class TestException : public std::runtime_error {
public:
    TestException(const std::string& message) : std::runtime_error(message) {}
};

void test_single_thread() {
    multithreaded_ds::Skiplist<int> skiplist;
    
    // Test basic operations
    std::cout << "Testing basic operations..." << std::endl;
    
    // Test add
    skiplist.add(5);
    skiplist.add(3);
    skiplist.add(7);
    skiplist.add(1);
    
    // Test search
    if (!skiplist.search(5)) {
        throw TestException("Failed to find value 5 in skiplist");
    }
    if (skiplist.search(2)) {
        throw TestException("Found non-existent value 2 in skiplist");
    }
    
    // Test erase
    if (!skiplist.erase(3)) {
        throw TestException("Failed to erase value 3 from skiplist");
    }
    if (skiplist.erase(4)) {
        throw TestException("Successfully erased non-existent value 4 from skiplist");
    }
    
    // Verify final state
    if (!skiplist.search(5) || !skiplist.search(7) || !skiplist.search(1)) {
        throw TestException("Expected values not found in final state");
    }
    if (skiplist.search(3)) {
        throw TestException("Erased value 3 still found in skiplist");
    }
}

void test_concurrent_operations() {
    multithreaded_ds::Skiplist<int> skiplist;
    const int num_threads = 4;
    const int operations_per_thread = 1000;
    std::vector<std::thread> threads;
    std::atomic<int> successful_adds{0};
    std::atomic<int> successful_erases{0};
    std::atomic<int> successful_searches{0};
    
    // Create threads that perform random operations
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&skiplist, i, &successful_adds, &successful_erases, &successful_searches]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 2);  // 0: add, 1: erase, 2: search
            std::uniform_int_distribution<> value_dis(0, 1000);
            
            for (int j = 0; j < operations_per_thread; ++j) {
                int value = value_dis(gen);
                int operation = dis(gen);
                
                switch (operation) {
                    case 0:  // add
                        skiplist.add(value);
                        successful_adds++;
                        break;
                    case 1:  // erase
                        if (skiplist.erase(value)) {
                            successful_erases++;
                        }
                        break;
                    case 2:  // search
                        if (skiplist.search(value)) {
                            successful_searches++;
                        }
                        break;
                }
            }
        });
    }
    
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify that the skiplist is in a consistent state
    if (successful_adds < successful_erases) {
        throw TestException("More elements erased than added");
    }
    
    std::cout << "\nConcurrent operations test results:" << std::endl;
    std::cout << "Successful adds: " << successful_adds << std::endl;
    std::cout << "Successful erases: " << successful_erases << std::endl;
    std::cout << "Successful searches: " << successful_searches << std::endl;
}

void test_concurrent_add_erase() {
    multithreaded_ds::Skiplist<int> skiplist;
    const int num_threads = 6;
    const int elements_per_thread = 500;
    std::vector<std::thread> threads;
    std::atomic<int> total_added{0};
    std::atomic<int> total_erased{0};
    
    // Create add threads
    for (int i = 0; i < num_threads/2; ++i) {
        threads.emplace_back([&skiplist, i, &total_added]() {
            for (int j = 0; j < elements_per_thread; ++j) {
                int value = i * elements_per_thread + j;
                skiplist.add(value);
                total_added++;
                // Add small delay to increase contention
                std::this_thread::sleep_for(std::chrono::microseconds(rand() % 5));
            }
        });
    }
    
    // Create erase threads
    for (int i = 0; i < num_threads/2; ++i) {
        threads.emplace_back([&skiplist, i, &total_erased]() {
            for (int j = 0; j < elements_per_thread; ++j) {
                int value = i * elements_per_thread + j;
                if (skiplist.erase(value)) {
                    total_erased++;
                }
                // Add small delay to increase contention
                std::this_thread::sleep_for(std::chrono::microseconds(rand() % 5));
            }
        });
    }
    
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify results
    if (total_erased > total_added) {
        throw TestException("More elements erased than added in concurrent add-erase test");
    }
    
    std::cout << "\nConcurrent add-erase test results:" << std::endl;
    std::cout << "Total elements added: " << total_added << std::endl;
    std::cout << "Total elements erased: " << total_erased << std::endl;
}

void test_rapid_concurrent_searches() {
    multithreaded_ds::Skiplist<int> skiplist;
    const int num_threads = 8;
    const int searches_per_thread = 10000;
    std::vector<std::thread> threads;
    std::atomic<int> successful_searches{0};
    
    // First, populate the skiplist with some values
    for (int i = 0; i < 1000; ++i) {
        skiplist.add(i);
    }
    
    // Create search threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&skiplist, i, &successful_searches]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 999);
            
            for (int j = 0; j < searches_per_thread; ++j) {
                int value = dis(gen);
                if (skiplist.search(value)) {
                    successful_searches++;
                }
            }
        });
    }
    
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify that we have a reasonable number of successful searches
    if (successful_searches < searches_per_thread * num_threads * 0.9) {
        throw TestException("Too few successful searches in rapid concurrent search test");
    }
    
    std::cout << "\nRapid concurrent searches test results:" << std::endl;
    std::cout << "Total successful searches: " << successful_searches << std::endl;
}

int main() {
    try {
        std::cout << "Testing single thread operations..." << std::endl;
        test_single_thread();
        
        std::cout << "\nTesting concurrent operations..." << std::endl;
        test_concurrent_operations();
        
        std::cout << "\nTesting concurrent add-erase scenario..." << std::endl;
        test_concurrent_add_erase();
        
        std::cout << "\nTesting rapid concurrent searches..." << std::endl;
        test_rapid_concurrent_searches();
        
        std::cout << "\nAll tests passed successfully!" << std::endl;
        return 0;
    } catch (const TestException& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}
