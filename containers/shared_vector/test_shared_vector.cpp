#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <climits>
#include <chrono>

#include "example.hpp"


void test_correctness(size_t n1, size_t n2, size_t n3, int seed = 123) {
    #ifndef NDEBUG
    SharedVector sh1(n1, n2, n3);
    size_t n = n1 + n2 + n3;
    std::vector<int> row(n1, 4), col(n2, -4);
    std::vector<double> val(n3, 4.5);
    std::fill_n(sh1.row, sh1.nrows, 4);
    std::fill_n(sh1.col, sh1.ncols, -4);
    std::fill_n(sh1.val, sh1.nvals, 4.5);

    std::mt19937 rng(seed); 
    std::uniform_real_distribution<> uni(0.0, 1.0);
    std::uniform_int_distribution<> num(INT_MIN, INT_MAX);
    std::vector<int> data_int(10 * n);
    std::vector<double> data_double(10 * n);

    for (size_t i = 0; i < n1; i++) assert(sh1.row[i] == row[i]);
    for (size_t i = 0; i < n2; i++) assert(sh1.col[i] == col[i]);
    for (size_t i = 0; i < n3; i++) assert(sh1.val[i] == val[i]);

    for (auto & x : data_int) {
        x = num(rng);
    }
    for (auto & x : data_double) {
        x = uni(rng);
    }
    auto test = [&](SharedVector & sh) {
        for (size_t i = 0; i < 2 * n; i++) {
            double v = uni(rng);
            size_t idx = num(rng);
            if (!data_int.empty() && v > 0.67) {
                sh.row[idx % n1] = data_int.back();
                row[idx % n1] = data_int.back();
                data_int.pop_back();
            } else if (!data_int.empty() && v > 0.33) {
                sh.col[idx % n2] = data_int.back();
                col[idx % n2] = data_int.back();
                data_int.pop_back();
            } else if (!data_double.empty()) {
                sh.val[idx % n3] = data_double.back();
                val[idx % n3] = data_double.back();
                data_double.pop_back();
            }
            for (size_t i = 0; i < n1; i++) assert(sh.row[i] == row[i]);
            for (size_t i = 0; i < n2; i++) assert(sh.col[i] == col[i]);
            for (size_t i = 0; i < n3; i++) assert(sh.val[i] == val[i]);
        } 
    };

    test(sh1);
    SharedVector sh2(std::move(sh1));
    test(sh2);
    sh1 = std::move(sh2);
    test(sh1);
    sh1.swap(sh2);
    test(sh2);
    sh1.swap(sh2);
    test(sh1);
    using std::swap;
    swap(sh1, sh2);
    #endif
}

int main() {
    test_correctness(50, 5, 45);
    test_correctness(76, 53, 5);
    test_correctness(8, 72, 64);
    std::cout << "OK" << std::endl;
}