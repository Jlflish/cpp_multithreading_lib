#include "../include/multithreaded_ds/hazard_pointer.hpp"
#include <atomic>
#include <vector>
#include <thread>
#include <algorithm>

namespace multithreaded_ds {

// Maximum number of hazard pointers per thread
constexpr size_t MAX_HAZARD_POINTERS = 2;
// Maximum number of retired nodes per thread
constexpr size_t MAX_RETIRED_NODES = 100;

// Thread-local storage for hazard pointers
thread_local std::vector<void*> hazard_pointers(MAX_HAZARD_POINTERS, nullptr);
// Thread-local storage for retired nodes
thread_local std::vector<std::pair<void*, std::function<void(void*)>>> retired_nodes;

hazard_pointer::hazard_pointer() noexcept {
    // Initialize thread-local storage if not already done
    if (hazard_pointers.empty()) {
        hazard_pointers.resize(MAX_HAZARD_POINTERS, nullptr);
    }
    if (retired_nodes.empty()) {
        retired_nodes.reserve(MAX_RETIRED_NODES);
    }
}

hazard_pointer::hazard_pointer(hazard_pointer&& other) noexcept {
    // Move constructor - no special handling needed
}

hazard_pointer& hazard_pointer::operator=(hazard_pointer&& other) noexcept {
    // Move assignment - no special handling needed
    return *this;
}

hazard_pointer::~hazard_pointer() {
    clear();
    // Clean up retired nodes
    for (auto& [ptr, deleter] : retired_nodes) {
        deleter(ptr);
    }
    retired_nodes.clear();
}

bool hazard_pointer::empty() const noexcept {
    return std::all_of(hazard_pointers.begin(), hazard_pointers.end(),
                      [](void* ptr) { return ptr == nullptr; });
}

bool hazard_pointer::protect(const std::atomic<void*>& src) noexcept {
    auto ptr = src.load(std::memory_order_acquire);
    if (!ptr) return false;
    
    // Find an empty slot in hazard pointers
    auto it = std::find(hazard_pointers.begin(), hazard_pointers.end(), nullptr);
    if (it == hazard_pointers.end()) return false;
    
    // Store the pointer in the hazard pointer
    *it = ptr;
    return true;
}

void hazard_pointer::clear() {
    std::fill(hazard_pointers.begin(), hazard_pointers.end(), nullptr);
}

template<class T>
void hazard_pointer::retire(void* ptr, std::function<void(void*)> deleter) noexcept {
    if (!ptr) return;
    
    // Add to retired nodes
    retired_nodes.emplace_back(ptr, deleter);
    
    // If we've reached the maximum number of retired nodes, try to reclaim some
    if (retired_nodes.size() >= MAX_RETIRED_NODES) {
        reclaim();
    }
}

void hazard_pointer::reclaim() {
    // Collect all hazard pointers from all threads
    std::vector<void*> all_hazard_pointers;
    for (const auto& ptr : hazard_pointers) {
        if (ptr) all_hazard_pointers.push_back(ptr);
    }
    
    // Remove retired nodes that are not protected by any hazard pointer
    auto it = std::remove_if(retired_nodes.begin(), retired_nodes.end(),
        [&](const auto& pair) {
            void* ptr = pair.first;
            return std::find(all_hazard_pointers.begin(), all_hazard_pointers.end(), ptr) == all_hazard_pointers.end();
        });
    
    // Delete the unprotected nodes
    for (auto curr = it; curr != retired_nodes.end(); ++curr) {
        curr->second(curr->first);
    }
    
    // Remove the deleted nodes from the retired list
    retired_nodes.erase(it, retired_nodes.end());
}

} // namespace multithreaded_ds 