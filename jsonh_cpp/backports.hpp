// <expected> backport from C++23 to C++20
#ifdef __cpp_lib_expected
#include <expected> // for std::expected
#define JSONH_CPP_EXPECTED std::expected
#define JSONH_CPP_UNEXPECTED std::unexpected
#else
#include "martinmoene/expected.hpp" // for nonstd::expected
#define JSONH_CPP_EXPECTED nonstd::expected
#define JSONH_CPP_UNEXPECTED nonstd::unexpected
#endif