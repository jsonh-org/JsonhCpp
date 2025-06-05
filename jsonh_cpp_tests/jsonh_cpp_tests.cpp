#define CATCH_CONFIG_MAIN
#include "catch2/catch_amalgamated.hpp" // for catch2

#include <cassert> // for assert
#include <iostream> // for std::cout
#include "../jsonh_cpp/jsonh_cpp.hpp" // for jsonh

using namespace jsonh_cpp;

/*
    Read Tests
*/

TEST_CASE("BasicObjectTest") {
    std::string jsonh = R"(
{
    "a": "b"
}
)";
    jsonh_reader reader(jsonh);
    std::vector<nonstd::expected<jsonh_token, std::string>> tokens = reader.read_element();

    for (const nonstd::expected<jsonh_token, std::string>& token : tokens) {
        REQUIRE(token);
    }
    REQUIRE(tokens[0].value().json_type == json_token_type::start_object);
    REQUIRE(tokens[1].value().json_type == json_token_type::property_name);
    REQUIRE(tokens[1].value().value == "a");
    REQUIRE(tokens[2].value().json_type == json_token_type::string);
    REQUIRE(tokens[2].value().value == "b");
    REQUIRE(tokens[3].value().json_type == json_token_type::end_object);
}

/*
    Parse Tests
*/

TEST_CASE("EscapeSequenceTest") {
    std::string jsonh = R"(
"\U0001F47D and \uD83D\uDC7D"
)";
    std::string element = jsonh_reader::parse_element<std::string>(jsonh).value();

    REQUIRE(element == "👽 and 👽");
}
TEST_CASE("QuotelessEscapeSequenceTest") {
    std::string jsonh = R"(
\U0001F47D and \uD83D\uDC7D
)";
    std::string element = jsonh_reader::parse_element<std::string>(jsonh).value();

    REQUIRE(element == "👽 and 👽");
}
TEST_CASE("MultiQuotedStringTest") {
    std::string jsonh = R"(
""""
  Hello! Here's a quote: ". Now a double quote: "". And a triple quote! """. Escape: \\\U0001F47D.
 """"
)";
    std::string element = jsonh_reader::parse_element<std::string>(jsonh).value();

    REQUIRE(element == " Hello! Here's a quote: \". Now a double quote: \"\". And a triple quote! \"\"\". Escape: \\👽.");
}
TEST_CASE("ArrayTest") {
    std::string jsonh = R"(
[
    1, 2,
    3
    4 5, 6
]
)";
    json element = jsonh_reader::parse_element(jsonh).value();

    REQUIRE(element.size() == 5);
    REQUIRE(element[0] == 1);
    REQUIRE(element[1] == 2);
    REQUIRE(element[2] == 3);
    REQUIRE(element[3] == "4 5");
    REQUIRE(element[4] == 6);
}
TEST_CASE("NumberParserTest") {
    REQUIRE((int)jsonh_number_parser::parse("1.2e3.4").value() == 3014);
}
TEST_CASE("BracelessObjectTest") {
    std::string jsonh = R"(
a: b
c : d
)";
    json element = jsonh_reader::parse_element(jsonh).value();

    REQUIRE(element.size() == 2);
    REQUIRE(element["a"] == "b");
    REQUIRE(element["c"] == "d");
}
TEST_CASE("CommentTest") {
    std::string jsonh = R"(
[
    1 # hash comment
        2 // line comment
        3 /* block comment */, 4
]
)";
    std::vector<int> element = jsonh_reader::parse_element<std::vector<int>>(jsonh).value();

    REQUIRE(element.size() == 4);
    REQUIRE(element[0] == 1);
    REQUIRE(element[1] == 2);
    REQUIRE(element[2] == 3);
    REQUIRE(element[3] == 4);
}

/*
    Edge Case Tests
*/

TEST_CASE("QuotelessStringStartingWithKeywordTest") {
    std::string jsonh = R"(
[nulla, null b, null]
)";
    std::vector<json> element = jsonh_reader::parse_element<std::vector<json>>(jsonh).value();

    REQUIRE(element.size() == 3);
    REQUIRE(element[0] == "nulla");
    REQUIRE(element[1] == "null b");
    REQUIRE(element[2] == nullptr);
}
TEST_CASE("BracelessObjectWithInvalidValueTest") {
    std::string jsonh = R"(
a: {
)";

    REQUIRE(!jsonh_reader::parse_element(jsonh));
}
TEST_CASE("NestedBracelessObjectTest") {
    std::string jsonh = R"(
[
    a: b
    c: d
]
)";

    REQUIRE(!jsonh_reader::parse_element<std::vector<std::string>>(jsonh));
}
TEST_CASE("QuotelessStringsLeadingTrailingWhitespaceTest") {
    std::string jsonh = R"(
[
    a b  , 
]
)";
    std::vector<std::string> element = jsonh_reader::parse_element<std::vector<std::string>>(jsonh).value();

    REQUIRE(element.size() == 1);
    REQUIRE(element[0] == "a b");
}
TEST_CASE("SpaceInQuotelessPropertyNameTest") {
    std::string jsonh = R"(
{
    a b: c d
}
)";
    json element = jsonh_reader::parse_element(jsonh).value();

    REQUIRE(element.size() == 1);
    REQUIRE(element["a b"] == "c d");
}
TEST_CASE("QuotelessStringsEscapeTest") {
    std::string jsonh = R"(
a: \"5
b: \\z
c: 5 \\
)";
    json element = jsonh_reader::parse_element(jsonh).value();

    REQUIRE(element.size() == 3);
    REQUIRE(element["a"] == "\"5");
    REQUIRE(element["b"] == "\\z");
    REQUIRE(element["c"] == "5 \\");
}
TEST_CASE("MultiQuotedStringsNoLastNewlineWhitespaceTest") {
    std::string jsonh = R"(
"""
  hello world  """
)";

    REQUIRE(jsonh_reader::parse_element<std::string>(jsonh).value() == "\n  hello world  ");
}
TEST_CASE("MultiQuotedStringsNoFirstWhitespaceNewlineTest") {
    std::string jsonh = R"(
"""  hello world
  """
)";

    REQUIRE(jsonh_reader::parse_element<std::string>(jsonh).value() == "  hello world\n  ");
}
TEST_CASE("QuotelessStringsEscapedLeadingTrailingWhitespaceTest") {
    std::string jsonh = R"(
\nZ\ \r
)";

    REQUIRE(jsonh_reader::parse_element<std::string>(jsonh).value() == "Z");
}
TEST_CASE("HexNumberWithETest") {
    std::string jsonh = R"(
0x5e3
)";

    REQUIRE(jsonh_reader::parse_element<int>(jsonh).value() == 0x5e3);

    std::string jsonh2 = R"(
0x5e+3
)";

    REQUIRE(jsonh_reader::parse_element<int>(jsonh2).value() == 5000);
}
TEST_CASE("NumberWithRepeatedUnderscoresTest") {
    std::string jsonh = R"(
100__000
)";

    REQUIRE(jsonh_reader::parse_element<int>(jsonh).value() == 100'000);
}
TEST_CASE("NumberWithUnderscoreAfterBaseSpecifierTest") {
    std::string jsonh = R"(
0b_100
)";

    REQUIRE(jsonh_reader::parse_element<int>(jsonh).value() == 0b100);
}