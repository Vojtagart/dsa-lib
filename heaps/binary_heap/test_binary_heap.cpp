#include <iostream>
#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include <functional>

#include "binary_heap.hpp"
#include <queue>

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
struct TestBinaryHeap {
    std::priority_queue<T, std::vector<T>, std::greater<T>> q;
    const T& top() const {
        return q.top();
    }
    bool empty() const {
        return q.empty();
    }
    size_t size() const {
        return q.size();
    }
    void push(const T& elem) {
        q.push(elem);
    }
    void push(T&& elem) {
        q.push(std::move(elem));
    }
    template<class... Args >
    void emplace(Args&&... args) {
        q.push(std::forward<Args>(args)...);
    }
    void pop() {
        q.pop();
    }
    void replace_top(const T & val) {
        q.pop();
        q.push(val);
    }
    void replace_top(T && val) {
        q.pop();
        q.push(std::move(val));
    }
};

template <typename T>
struct Tester {
    TestBinaryHeap<T> r;
    dsa::BinaryHeap<T> s;
    void push(const T& elem) {
        r.push(elem);
        s.push(elem);
    }
    template<class... Args >
    void emplace(Args&&... args) {
        r.push(std::forward<Args>(args)...);
        s.push(std::forward<Args>(args)...);
    }
    void pop() {
        r.pop();
        s.pop();
    }
    void replace_top(const T & val) {
        r.replace_top(val);
        s.replace_top(val);
    }
    void check() const {
        assert(r.size() == s.size());
        assert(r.empty() == s.empty());
        if (!r.empty()) assert(r.top() == s.top());
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

    for (size_t i = 0; i < ops; i++) {
        if (uni(rng) > add_prob && !t.empty()) {
            t.pop();
        } else if (t.size() < max_elems) {
            T val = factory();
            t.push(val);
        }
        t.check();
    }
    while (!t.empty()) {
        t.pop();
        t.check();
    }

    for (size_t i = 0; i < ops; i++) {
        if (uni(rng) > add_prob / 2 && !t.empty()) {
            t.pop();
        } else if (uni(rng) > add_prob / 2 && !t.empty()) {
            T val = factory();
            t.replace_top(val);
        } else if (t.size() < max_elems)  {
            T val = factory();
            t.push(val);
        }
        t.check();
    }
    while (!t.empty()) {
        t.pop();
        t.check();
    }
}

void test_dummy() {
    dsa::BinaryHeap<Dummy<double>> q;
    std::mt19937 rng(1450); 
    std::uniform_real_distribution<> uni(0.0, 1.0);

    for (size_t i = 0; i < 1'000; i++) {
        q.push(Dummy(uni(rng)));
    }
    for (size_t i = 0; i < 1'000; i++) {
        q.replace_top(Dummy(uni(rng)));
        q.push(Dummy(uni(rng)));
    }
    for (size_t i = 0; i < 2'000; i++) {
        q.pop();
    }
    for (size_t i = 0; i < 1'000; i++) {
        q.emplace(uni(rng));
    }
    using std::swap;
    dsa::BinaryHeap<Dummy<double>> q2;
    q2.emplace(10.);
    swap(q, q2);
    q.emplace(5);
    dsa::BinaryHeap<Dummy<double>> q3(std::move(q));
    dsa::BinaryHeap<Dummy<double>> q4;
    q4 = std::move(q2);
    q.reserve(100);
    q.swap(q2);
}

void test_heapify() {
    std::vector<int> a(1'000'000);
    std::mt19937 rng(143); 
    std::uniform_int_distribution<> uni(0, 500'000);
    for (auto & x : a) {
        x = uni(rng);
    }
    dsa::BinaryHeap<int> q(a);
    sort(a.begin(), a.end());
    for (auto x : a) {
        assert(x == q.min());
        q.pop();
    }

    dsa::BinaryHeap<int> q2(a.rbegin(), a.rend());
    for (auto x : a) {
        assert(x == q2.min());
        q2.pop();
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
    test_corectness<double>(fact1, 10'000'000, -size_t(1), 0.67, 120);
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