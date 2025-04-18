// <expected> backport
#ifdef __cpp_lib_expected
#include <expected> // for std::expected
#define JSONH_CPP_EXPECTED std::expected
#define JSONH_CPP_UNEXPECTED std::unexpected
#endif
#ifndef __cpp_lib_expected
#include "martinmoene/expected.hpp" // for nonstd::expected
#define JSONH_CPP_EXPECTED nonstd::expected
#define JSONH_CPP_UNEXPECTED nonstd::unexpected
#endif