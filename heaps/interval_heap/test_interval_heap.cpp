#include <iostream>
#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include <functional>

#include "interval_heap.hpp"
#include <set>

template <typename T>
struct Dummy {
    T val;
    Dummy() = delete;
    Dummy(const T & val) : val(val) {}
    Dummy(T && val) : val(std::move(val)) {}
    Dummy(const Dummy& other) = delete;
    Dummy(Dummy&& other) : val(std::move(other.val)) {}
    Dummy& operator = (const Dummy& other) = delete;
    Dummy& operator = (Dummy&& other) {
        val = std::move(other.val);
        return *this;
    }
    bool operator < (const Dummy & other) const {
        return val < other.val;
    }
}; 

template <typename T>
struct TestIntervalHeap {
    std::multiset<T> q;
    const T& min() const {
        return *q.begin();
    }
    const T& max() const {
        return *q.rbegin();
    }
    bool empty() const {
        return q.empty();
    }
    size_t size() const {
        return q.size();
    }
    void push(const T& elem) {
        q.insert(elem);
    }
    void push(T&& elem) {
        q.insert(std::move(elem));
    }
    template<class... Args >
    void emplace(Args&&... args) {
        q.emplace(std::forward<Args>(args)...);
    }
    void pop_min() {
        q.erase(q.begin());
    }
    void pop_max() {
        q.erase(std::prev(q.end()));
    }
    void replace_min(const T & val) {
        pop_min();
        push(val);
    }
    void replace_min(T && val) {
        pop_min();
        push(val);
    }
    void replace_max(const T & val) {
        pop_max();
        push(val);
    }
    void replace_max(T && val) {
        pop_max();
        push(val);
    }
};

template <typename T>
struct Tester {
    TestIntervalHeap<T> r;
    dsa::IntervalHeap<T> s;
    void push(const T& elem) {
        r.push(elem);
        s.push(elem);
    }
    template<class... Args >
    void emplace(Args&&... args) {
        r.push(std::forward<Args>(args)...);
        s.push(std::forward<Args>(args)...);
    }
    void pop_min() {
        r.pop_min();
        s.pop_min();
    }
    void pop_max() {
        r.pop_max();
        s.pop_max();
    }
    void replace_min(const T & val) {
        r.replace_min(val);
        s.replace_min(val);
    }
    void replace_max(const T & val) {
        r.replace_max(val);
        s.replace_max(val);
    }
    void check() const {
        assert(r.size() == s.size());
        assert(r.empty() == s.empty());
        if (!r.empty()) {
            assert(r.min() == s.min());
            assert(r.max() == s.max());
        }
    }
    bool empty() const {
        return r.empty();
    }
    size_t size() const {
        return r.size();
    }
};

/**
 * Several randomized validity checks and speed checks,
 * both compared to std::priority_queue
 */

using chrono_ns = std::chrono::nanoseconds;

template <typename T>
void test_corectness(std::function<T()> factory, size_t ops = 1'000'000, size_t max_elems = -size_t(1), double add_prob = 0.67, size_t seed = 123) {
    std::mt19937 rng(seed); 
    std::uniform_real_distribution<> uni(0.0, 1.0);
    Tester<T> t;

    const double pop_min = (1.0 + add_prob) / 2;
    for (size_t i = 0; i < ops; i++) {
        double num = uni(rng);
        if (num > pop_min && !t.empty()) {
            t.pop_min();
        } else if (num > add_prob && !t.empty()) {
            t.pop_max();
        } else if (t.size() < max_elems) {
            T val = factory();
            t.push(val);
        }
        t.check();
    }
    while (!t.empty()) {
        t.pop_max();
        t.check();
    }

    double pop = add_prob + 2 * (1.0 - add_prob) / 3;
    double rep_min = add_prob + 1 * (1.0 - add_prob) / 3;
    double rep_max = add_prob + 0 * (1.0 - add_prob) / 3;
    for (size_t i = 0; i < ops; i++) {
        double num = uni(rng);
        if (num > pop && !t.empty()) {
            if (uni(rng) > 0.5) t.pop_min();
            else t.pop_max();
        } else if (num > rep_min && !t.empty()) {
            T val = factory();
            t.replace_min(val);
        } else if (num > rep_max && !t.empty()) {
            T val = factory();
            t.replace_max(val);
        } else if (t.size() < max_elems)  {
            T val = factory();
            t.push(val);
        }
        t.check();
    }
    while (!t.empty()) {
        t.pop_min();
        t.check();
    }
}

void test_dummy() {
    dsa::IntervalHeap<Dummy<double>> q;
    std::mt19937 rng(1450); 
    std::uniform_real_distribution<> uni(0.0, 1.0);

    for (size_t i = 0; i < 1'000; i++) {
        q.push(Dummy(uni(rng)));
    }
    for (size_t i = 0; i < 1'000; i++) {
        q.replace_min(Dummy(uni(rng)));
        q.replace_max(Dummy(uni(rng)));
        q.push(Dummy(uni(rng)));
    }
    for (size_t i = 0; i < 1'000; i++) {
        q.pop_min();
        q.pop_max();
    }
    for (size_t i = 0; i < 1'000; i++) {
        q.emplace(uni(rng));
    }
}

void test_heapify() {
    std::vector<int> a(1'000'000);
    std::mt19937 rng(143); 
    std::uniform_int_distribution<> uni(0, 500'000);
    for (auto & x : a) {
        x = uni(rng);
    }
    dsa::IntervalHeap<int> q(a);
    sort(a.begin(), a.end());
    for (auto x : a) {
        assert(x == q.min());
        q.pop_min();
    }

    dsa::IntervalHeap<int> q2(a.rbegin(), a.rend());
    for (auto it = a.rbegin(); it != a.rend(); it++) {
        assert(*it == q2.max());
        q2.pop_max();
    }
}

int main() {
    #ifndef NDEBUG
    std::cout << "-------------------------" << std::endl;
    std::mt19937 rng(100);
    std::uniform_real_distribution<> uni(0.0, 1.0);
    auto fact1 = [&]() {
        return uni(rng);
    };
    test_corectness<double>(fact1, 5'000'000, -size_t(1), 0.67, 120);
    std::cout << "Correctness 1 finished" << std::endl;
    test_corectness<double>(fact1, 1'000'000, 20, 0.4, 115);
    std::cout << "Correctness 2 finished" << std::endl;
    std::uniform_int_distribution<> alpha('a', 'z');
    std::uniform_int_distribution<> len(0, 40);
    auto fact2 = [&]() {
        char c = alpha(rng);
        int l = len(rng);
        return std::string(l, c);
    };
    test_corectness<std::string>(fact2, 1'000'000, -size_t(1), 0.67, 69);
    std::cout << "Correctness 3 finished" << std::endl;
    test_corectness<std::string>(fact2, 1'000'000, 20, 0.4, 452);
    std::cout << "Correctness 4 finished" << std::endl;
    test_dummy();
    std::cout << "Dummy test finished" << std::endl;
    test_heapify();
    std::cout << "Heapify test finished" << std::endl;
    std::cout << "-------------------------" << std::endl;
    #else
    std::cout << "Correctness checks skipped (#define NDEBUG)" << std::endl;
    #endif
}