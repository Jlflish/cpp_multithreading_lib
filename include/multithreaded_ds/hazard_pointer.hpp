#ifndef HAZARD_POINTER_HPP
#define HAZARD_POINTER_HPP

#include <atomic>
#include <functional>
#include <vector>
#include <memory>

namespace multithreaded_ds {

class hazard_pointer {
public:
    hazard_pointer() noexcept;
    hazard_pointer(hazard_pointer&&) noexcept;
    hazard_pointer& operator=(hazard_pointer&&) noexcept;
    ~hazard_pointer();

    bool empty() const noexcept;
    
    bool protect(const std::atomic<void*>& src) noexcept;
    
    void clear();
    
    void retire(void* ptr, std::function<void(void*)> deleter) noexcept;

private:
    void reclaim();
    
    // Maximum number of hazard pointers per thread
    static constexpr size_t MAX_HAZARD_POINTERS = 2;
    // Maximum number of retired nodes per thread
    static constexpr size_t MAX_RETIRED_NODES = 100;
    
    // Thread-local storage for hazard pointers
    static thread_local std::vector<void*> hazard_pointers;
    // Thread-local storage for retired nodes
    static thread_local std::vector<std::pair<void*, std::function<void(void*)>>> retired_nodes;
};

} // namespace multithreaded_ds

#endif // HAZARD_POINTER_HPP