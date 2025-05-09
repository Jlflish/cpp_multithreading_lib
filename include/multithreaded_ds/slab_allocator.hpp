#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>

namespace multithreaded_ds {
class slab {
public:
    static constexpr size_t page_size = 4096;

    static slab* create(size_t block_size) {
        size_t total = sizeof(slab) + page_size;
        void* mem = ::operator new(total, std::align_val_t{page_size});
        return new(mem) slab(block_size);
    }

    void destroy() {
        this->~slab();
        ::operator delete(this, std::align_val_t{page_size});
    }

    slab(const slab&) = delete;
    slab(slab&&) = delete;
    slab& operator=(const slab&) = delete;
    slab& operator=(slab&&) = delete;

    explicit slab(size_t block_size) :
    m_block_size(block_size),
    m_valid_page(page_size / block_size),
    m_head(nullptr) {
        assert(block_size != 0 && page_size % block_size == 0);
        assert(block_size >= sizeof(void*) && block_size % alignof(void*) == 0);
        m_data_set = reinterpret_cast<char*>(this) + sizeof(slab);
        char* p = m_data_set;
        for (size_t i = 0; i + 1 < m_valid_page; i++) {
            void* next = p + m_block_size;
            *reinterpret_cast<void**>(p) = next;
            p += m_block_size;
        }
        *reinterpret_cast<void**>(p) = nullptr;
        m_head = m_data_set;
    }

    ~slab() = default;

    void* allocate() {
        if (m_valid_page == 0) {
            return nullptr;
        }
        void* p = m_head;
        m_head = *reinterpret_cast<void**>(p);
        --m_valid_page;
        return p;
    }

    void deallocate(void *p) {
        *reinterpret_cast<void**>(p) = m_head;
        m_head = p;
        ++m_valid_page;
    }

    size_t size() const noexcept { return m_valid_page; }
    bool empty() const noexcept { return m_valid_page == 0; }
    bool full() const noexcept { return m_valid_page == page_size / m_block_size; }

private:
    char* m_data_set;
    void* m_head;
    size_t m_block_size;
    size_t m_valid_page;
};

class slab_allocator {
public:
    explicit slab_allocator(size_t block_size) :
    m_block_size(block_size) {

    }
    ~slab_allocator() {
        for (auto i : local_threads) {
            i->destroy();
        }
        std::lock_guard<std::mutex> lock(global_mtx);
        for (auto i : global_threads) {
            i->destroy();
        }
    }
    void* allocate() {
        if (!local_threads.empty()) {
            slab *S = local_threads.back();
            void *res = S->allocate();
            if (S->empty()) {
                local_threads.pop_back();
            }
            return res;
        }
        {
            std::lock_guard<std::mutex> lock(global_mtx);
            if (!global_threads.empty()) {
                push_local(global_threads.back());
                global_threads.pop_back();
            }
        }
        if (local_threads.empty()) {
            push_local(slab::create(m_block_size));
        }
        auto S = pop_local();
        auto p = S->allocate();
        if (!S->empty()) {
            push_local(S);
        }
        return p;
    }
    void deallocate(void *p) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        uintptr_t base = addr & ~(slab::page_size - 1);
        slab* S = reinterpret_cast<slab*>(base);
        S->deallocate(p);
        if (S->full()) {
            auto it = std::find(local_threads.begin(), local_threads.end(), S);
            if (it != local_threads.end()) {
                local_threads.erase(it);
            }
            std::lock_guard<std::mutex> lock(global_mtx);
            global_threads.push_back(S);
        }
     }
private:
    slab* pop_local() {
        auto p = local_threads.back();
        local_threads.pop_back();
        return p;
    }
    void push_local(slab* p) {
        local_threads.push_back(p);    
    }

    size_t m_block_size;
    thread_local static std::vector<slab*> local_threads;
    static std::vector<slab*> global_threads;
    static std::mutex global_mtx;
};

thread_local std::vector<slab*> slab_allocator::local_threads;
std::vector<slab*> slab_allocator::global_threads;
std::mutex slab_allocator::global_mtx;

} // namespace multithreaded_ds 