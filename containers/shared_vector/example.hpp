#include <type_traits>
#include <algorithm>


struct SharedVector {

    int* row;
    int* col;
    double* val;
    size_t nrows;
    size_t ncols;
    size_t nvals;

    SharedVector(size_t nrows, size_t ncols, size_t nvals) : nrows(nrows), ncols(ncols), nvals(nvals) {
        size_t row_begin = 0;
        size_t col_begin = align<int>(row_begin + sizeof(int) * nrows);
        size_t val_begin = align<double>(col_begin + sizeof(int) * ncols);
        size_t total = val_begin + sizeof(double) * nvals;
        unsigned char* buffer = new unsigned char[total];
        row = reinterpret_cast<int*>(buffer + row_begin);
        col = reinterpret_cast<int*>(buffer + col_begin);
        val = reinterpret_cast<double*>(buffer + val_begin);
    }
    ~SharedVector() {
        if(row)
            delete[] reinterpret_cast<unsigned char*>(row);
    }
    SharedVector(const SharedVector& other) = delete;
    constexpr SharedVector(SharedVector&& other) : row(other.row), col(other.col), val(other.val), nrows(other.nrows), ncols(other.ncols), nvals(other.nvals) {
        other.reset();
    }
    SharedVector& operator = (const SharedVector& other) = delete;
    constexpr SharedVector& operator = (SharedVector&& other) {
        swap(other);
        return *this;
    }
    constexpr void swap(SharedVector& other) noexcept {
        std::swap(row, other.row);
        std::swap(col, other.col);
        std::swap(val, other.val);
        std::swap(nrows, other.nrows);
        std::swap(ncols, other.ncols);
        std::swap(nvals, other.nvals);
    }
    friend constexpr void swap(SharedVector& lhs, SharedVector& rhs) noexcept {
        lhs.swap(rhs);
    }

private:
    template <typename U>
    static constexpr size_t align(size_t idx) noexcept {
        return (idx + alignof(U) - 1) / alignof(U) * alignof(U);
    }
    constexpr void reset() {
        row = nullptr;
        col = nullptr;
        val = nullptr;
        nrows = 0;
        ncols = 0;
        nvals = 0;
    }
};
