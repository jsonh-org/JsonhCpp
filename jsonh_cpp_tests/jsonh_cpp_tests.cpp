#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp" // for catch2

#include <cassert> // for assert
#include <iostream> // for std::cout
#include "../jsonh_cpp/jsonh_cpp.hpp" // for jsonh

using namespace jsonh;

const std::string unicode_character = "\u79C1"; // 私

TEST_CASE("Basic Test") {
    std::cout << jsonh_reader("'hello'").parse_element().value() << '\n';
    /*for (std::expected<jsonh_token, std::string> v : jsonh_reader("'hello'").read_element()) {
        std::cout << (int)v.value().json_type << '\n';
        std::cout << v.value().value << '\n';
    }*/


    std::string jsonh = R"(
6 ab a
)";
    std::string element = jsonh::jsonh_reader(jsonh).parse_element<std::string>().value();
    std::cout << element << '\n';
}

TEST_CASE("Unicode Escape Sequences") {
    REQUIRE(jsonh_reader("'\\u00E7'").parse_element().value() == "ç");
    REQUIRE(jsonh_reader("'\\xE7'").parse_element().value() == "ç");
    REQUIRE(jsonh_reader("'\\U0001F47D'").parse_element().value() == "👽");
}

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
    for (std::expected<jsonh_token, std::string>& token : reader.read_element()) {
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
    for (std::expected<jsonh_token, std::string>& token : reader.read_element()) {
        if (!token) {
            found_error = true;
            break;
        }
    }
    if (!found_error) {
        REQUIRE(false);
    }
}

TEST_CASE("string") {
    jsonh_reader reader(unicode_character);
    REQUIRE(reader.parse_element<std::string>() == unicode_character);
}

TEST_CASE("qstring") {
    jsonh_reader reader("a b ");
    REQUIRE(reader.parse_element<std::string>() == "a b");
}

/*TEST_CASE("u8string") {
    jsonh_reader reader(utf8::utf32tou8(utf8::utf8to32(std::string(unicode_character))));
    REQUIRE(reader.parse_element<std::string>() == unicode_character);
}

TEST_CASE("u16string") {
    jsonh_reader reader(utf8::utf8to16(std::string(unicode_character)));
    REQUIRE(reader.parse_element<std::string>() == unicode_character);
}

TEST_CASE("u32string") {
    jsonh_reader reader(utf8::utf8to32(std::string(unicode_character)));
    REQUIRE(reader.parse_element<std::string>() == unicode_character);
}*/