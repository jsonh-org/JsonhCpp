// <expected> backport
#ifdef __cpp_lib_expected
#include <expected> // for std::expected
#define JSONH_CPP_EXPECTED std::expected
#define JSONH_CPP_UNEXPECTED std::unexpected
#endif
#ifndef __cpp_lib_expected
#include "zeus/expected.hpp" // for zeus::expected
#define JSONH_CPP_EXPECTED zeus::expected
#define JSONH_CPP_UNEXPECTED zeus::unexpected
#endif