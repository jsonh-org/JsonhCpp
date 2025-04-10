#include <cassert> // for assert
#include <iostream> // for std::cout
#include "../jsonh_cpp/jsonh_cpp.hpp" // for jsonh_cpp

int main() {
    assert((new jsonh_reader(new std::string("aaa")))->read().has_error() == false);

    std::cout << "All done!";
}