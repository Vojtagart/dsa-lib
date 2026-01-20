#include <iostream>
#include <utility>
#include <type_traits>
#include <string>
#include <set>
#include <vector>
#include <algorithm>


struct Elem {
    std::string type, name, len;
};

/**
 * ===========================================
 *   Generator for SharedVector - a class of trivial
 *   types stored in one continous array for memory locality
 *   less heap calls
 * ===========================================
 */


/**
 * @brief Set class name and tab width
 */
std::string class_name = "SharedVector";
std::string tab = "    ";
std::string tabtab = tab + tab;

/**
 * @brief Set struct attributes here
 * 
 * Each attribute has to be in format:
 * Type, Name, Number of those elements
 */
std::vector<Elem> elems {
    Elem{"int", "row", "nrows"},
    Elem{"int", "col", "ncols"},
    Elem{"double", "val", "nvals"},
};

std::vector<std::string> types, sizes;

std::string beg(const std::string & s) {
    return s + "_begin";
}

void print_body() {
    for (auto & e : elems) {
        std::cout << tab << e.type << "* " << e.name << ";\n";
    }
    for (auto s : sizes) {
        std::cout << tab << "size_t " << s << ";\n";
    }
}

void print_init() {
    // Constructor definition
    std::cout << tab << class_name << "(";
    for (size_t i = 0; i < sizes.size(); i++) {
        if (i != 0) std::cout << ", ";
        std::cout << "size_t " << sizes[i];
    }
    // Initialization
    std::cout << ") : ";
    for (size_t i = 0; i < sizes.size(); i++) {
        if (i != 0) std::cout << ", ";
        std::cout << sizes[i] << "(" << sizes[i] << ")";
    }
    std::cout << " {\n";
    // Begins calculation
    for (size_t i = 0; i < elems.size(); i++) {
        auto & e = elems[i];
        std::cout << tabtab << "size_t " << beg(e.name) << " = ";
        if (i == 0) {
            std::cout << 0 << ";\n";
            continue;
        }
        auto & pe = elems[i - 1];
        std::cout << "align<" << e.type << ">(" << beg(pe.name) << " + sizeof(" << pe.type << ") * " << pe.len << ");\n";
    }
    auto & last = elems.back();
    std::cout << tabtab << "size_t total = " << beg(last.name) << " + sizeof(" << last.type << ") * " << last.len << ";\n";
    // buffer allocation
    std::cout << tabtab << "unsigned char* buffer = new unsigned char[total];\n";
    // Pointer setting
    for (auto & e : elems) {
        std::cout << tabtab << e.name << " = reinterpret_cast<" << e.type << "*>(buffer + " << beg(e.name) << ");\n";
    }
    std::cout << tab << "}\n"; 
}

void print_copyconst() {
    std::cout << tab << class_name << "(const " << class_name << "& other) = delete;\n";
    std::cout << tab << "constexpr " << class_name << "(" << class_name << "&& other) : ";
    // Initialization
    for (size_t i = 0; i < elems.size(); i++) {
        if (i != 0) std::cout << ", ";
        std::cout << elems[i].name << "(other." << elems[i].name << ")";
    }
    for (size_t i = 0; i < sizes.size(); i++) {
        std::cout << ", " << sizes[i] << "(other." << sizes[i] << ")";
    }
    std::cout
    << " {\n"
    << tabtab << "other.reset();\n"
    << tab << "}\n";
}

void print_dest() {
    std::cout
    << tab << "~" << class_name << "() {\n"
    << tabtab << "if(" << elems.begin()->name << ")\n"
    << tabtab << tab << "delete[] reinterpret_cast<unsigned char*>(" << elems.begin()->name << ");\n"
    << tab << "}\n";
}

void print_assignment() {
    std::cout
    << tab << class_name << "& operator = (const " << class_name << "& other) = delete;\n"
    << tab << "constexpr " << class_name << "& operator = (" << class_name << "&& other) {\n"
    << tabtab << "swap(other);\n"
    << tabtab << "return *this;\n"
    << tab << "}\n";
}

void print_align() {
    std::cout
    << tab << "template <typename U>\n"
    << tab << "static constexpr size_t align(size_t idx) noexcept {\n"
    << tabtab << "return (idx + alignof(U) - 1) / alignof(U) * alignof(U);\n"
    << tab << "}\n";
}

void print_reset() {
    std::cout << tab << "constexpr void reset() {\n";
    for (auto & e : elems) {
        std::cout << tabtab << e.name << " = nullptr;\n";
    }
    for (auto s : sizes) {
        std::cout << tabtab << s << " = 0;\n";
    }
    std::cout << tab << "}\n";
}

void print_swap() {
    std::cout << tab << "constexpr void swap(" << class_name << "& other) noexcept {\n";
    for (auto & e : elems) {
        std::cout << tabtab << "std::swap(" << e.name << ", other." << e.name << ");\n";
    }
    for (auto s : sizes) {
        std::cout << tabtab << "std::swap(" << s << ", other." << s << ");\n";
    }
    std::cout << tab << "}\n";

    std::cout
    << tab << "friend constexpr void swap(" << class_name << "& lhs, " << class_name << "& rhs) noexcept {\n"
    << tabtab << "lhs.swap(rhs);\n"
    << tab << "}\n";
}

void print_headers() {
    std::cout
    << "#include <type_traits>\n"
    << "#include <algorithm>\n"
    << "\n\n";
}

void print_req() {
    std::cout << "requires(";
    for (size_t i = 0; i < types.size(); i++) {
        if (i != 0) std::cout << " && ";
        std::cout << "std::is_trivial_v<" << types[i] << ">";
    }
    std::cout << ")\n";
}

int main() {
    for (auto & e : elems) {
        if (std::find(types.begin(), types.end(), e.type) == types.end())
            types.push_back(e.type);
        if (std::find(sizes.begin(), sizes.end(), e.len) == sizes.end())
            sizes.push_back(e.len);
    }

    print_headers();
    print_req();

    std::cout << "struct " << class_name << " {\n\n";
    print_body();
    std::cout << '\n';
    print_init();
    print_dest();
    print_copyconst();
    print_assignment();
    print_swap();
    std::cout << "\nprivate:\n";
    print_align();
    print_reset();

    std::cout << "};\n";
}
