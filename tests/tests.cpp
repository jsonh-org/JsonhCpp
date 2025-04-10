#include <cassert> // for assert
#include <iostream> // for std::cout
#include "../jsonh_cpp/jsonh_cpp.hpp" // for jsonh

using namespace jsonh;

int main() {
    jsonh_reader reader("aaa");

    for (result<void, jsonh_error> token : reader.read_element()) {
        assert(token.has_error() == false);
    }

    std::cout << "All done!";
}