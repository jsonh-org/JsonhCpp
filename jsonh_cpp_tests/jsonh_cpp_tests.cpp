#include <cassert> // for assert
#include <iostream> // for std::cout
#include "../jsonh_cpp/jsonh_cpp.hpp" // for jsonh

using namespace jsonh;

static void assert_all_values(std::vector<std::expected<jsonh_token, std::string_view>> tokens) {
    for (std::expected<jsonh_token, std::string_view>& token : tokens) {
        assert(token);
    }
}
static void assert_any_errors(std::vector<std::expected<jsonh_token, std::string_view>> tokens) {
    for (std::expected<jsonh_token, std::string_view>& token : tokens) {
        if (!token) {
            return;
        }
    }
    assert(false);
}

int main() {
    // Comments
    {
        jsonh_reader reader(R"(
// line comment
/* block comment */
/* multiline
block comment */
)");
        auto tokens = reader.read_element();
        assert(tokens[0].value().value == " line comment");
        assert(tokens[1].value().value == " block comment ");
        assert(tokens[2].value().value == R"( multiline
block comment )");
    }
    // Test 1
    {
        jsonh_reader reader(R"(
// hello
/* hi */
aaa
)");
        assert_all_values(reader.read_element());
    }
    // Test 2
    {
        jsonh_reader reader(R"(
// hello
/w hi
aaa
)");
        assert_any_errors(reader.read_element());
    }
    // End
    std::cout << "All done!";
}