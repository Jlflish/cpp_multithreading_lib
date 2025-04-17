#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <random>
#include <mutex>

namespace multithreaded_ds {

template <typename T, int P = 20>
class Skiplist {
private:
    struct Node {
        std::vector<std::shared_ptr<Node>> next;
        std::optional<T> value;
        Node(std::optional<T> val = std::nullopt)
            : value(val), next(P, nullptr) {}
    };

    std::shared_ptr<Node> m_head;
    std::mt19937_64 rng{std::random_device{}()};
    
    bool random_level_check() {
        return rng() % 2 == 0;
    }

    std::mutex mtx;

public:
    Skiplist() {
        m_head = std::make_shared<Node>();
    }

    bool search(T target) {
        std::lock_guard<std::mutex> lock(mtx);
        auto cur = m_head;
        for (int level = P - 1; level >= 0; --level) {
            while (cur->next[level] && cur->next[level]->value.value() < target) {
                cur = cur->next[level];
            }
        }
        cur = cur->next[0];
        return cur && cur->value.has_value() && cur->value.value() == target;
    }

    void add(T num) {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<std::shared_ptr<Node>> update(P, nullptr);
        auto cur = m_head;

        for (int level = P - 1; level >= 0; --level) {
            while (cur->next[level] && cur->next[level]->value.value() < num) {
                cur = cur->next[level];
            }
            update[level] = cur;
        }

        auto newNode = std::make_shared<Node>(num);
        for (int level = 0; level < P; ++level) {
            newNode->next[level] = update[level]->next[level];
            update[level]->next[level] = newNode;
            if (!random_level_check()) break;
        }
    }

    bool erase(T num) {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<std::shared_ptr<Node>> update(P, nullptr);
        auto cur = m_head;

        for (int level = P - 1; level >= 0; --level) {
            while (cur->next[level] && cur->next[level]->value.value() < num) {
                cur = cur->next[level];
            }
            update[level] = cur;
        }

        auto target = cur->next[0];
        if (!target || !target->value.has_value() || target->value.value() != num) {
            return false;
        }

        for (int level = P - 1; level >= 0; --level) {
            if (update[level]->next[level] == target) {
                update[level]->next[level] = target->next[level];
            }
        }
        return true;
    }

    friend std::ostream& operator<<(std::ostream& out, const Skiplist& list) {
        auto cur = list.m_head->next[0];
        while (cur) {
            if (cur->value.has_value()) {
                out << cur->value.value() << ' ';
            }
            cur = cur->next[0];
        }
        return out;
    }
};

}  // namespace multithreaded_ds