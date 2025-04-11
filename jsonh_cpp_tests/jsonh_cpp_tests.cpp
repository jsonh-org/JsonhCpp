#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp" // for catch2

#include <cassert> // for assert
#include <iostream> // for std::cout
#include "../jsonh_cpp/jsonh_cpp.hpp" // for jsonh

using namespace jsonh;

TEST_CASE("Comments") {
    jsonh_reader reader(R"(
// line comment
/* block comment */
/* multiline
block comment */
)");
    auto tokens = reader.read_element();
    REQUIRE(tokens[0].value().value == " line comment");
    REQUIRE(tokens[1].value().value == " block comment ");
    REQUIRE(tokens[2].value().value == R"( multiline
block comment )");
}

TEST_CASE("NumberParserTest") {
    REQUIRE((int)jsonh_number_parser::parse("1.2e3.4").value() == 3014);
}

TEST_CASE("Test 1") {
    jsonh_reader reader(R"(
// hello
/* hi */
aaa
)");
    for (std::expected<jsonh_token, std::string_view>& token : reader.read_element()) {
        REQUIRE(token);
    }
}

TEST_CASE("Test 2") {
    jsonh_reader reader(R"(
// hello
/w hi
aaa
)");
    bool found_error = false;
    for (std::expected<jsonh_token, std::string_view>& token : reader.read_element()) {
        if (!token) {
            found_error = true;
            break;
        }
    }
    if (!found_error) {
        REQUIRE(false);
    }
}