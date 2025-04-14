#pragma once

#include <string> // for std::string
#include <sstream> // for std::istringstream
#include <set> // for std::set
#include <stack> // for std::stack
#include <expected> // for std::expected
#include "nlohmann/json.hpp" // for nlohmann::json
#define UTF_CPP_CPLUSPLUS 202302L // for utf8 (C++23)
#include "utf8cpp/utf8.h" // for utf8
#include "jsonh_token.hpp" // for jsonh::jsonh_token
#include "jsonh_reader_options.hpp" // for jsonh::jsonh_reader_options
#include "jsonh_number_parser.hpp" // for jsonh::jsonh_number_parser

using namespace nlohmann;

namespace jsonh {

/// <summary>
/// A reader that reads tokens from an input stream encoded as UTF-8.
/// </summary>
class jsonh_reader final {
public:
    /// <summary>
    /// The stream to read characters from.
    /// </summary>
    std::unique_ptr<std::istream> stream;
    /// <summary>
    /// The options to use when reading JSONH.
    /// </summary>
    jsonh_reader_options options;
    /// <summary>
    /// The number of characters read from <see cref="stream"/>.
    /// </summary>
    long char_counter = 0;

    /// <summary>
    /// Constructs a reader that reads JSONH from a stream.
    /// </summary>
    jsonh_reader(std::unique_ptr<std::istream> stream, jsonh_reader_options options = jsonh_reader_options()) noexcept {
        this->stream = std::move(stream);
        this->options = options;
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a string.
    /// </summary>
    jsonh_reader(const std::string& string, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader(std::make_unique<std::istringstream>(string), options) {
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a string.
    /// </summary>
    jsonh_reader(const char* string, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader(std::string(string), options) {
    }

    /// <summary>
    /// Frees the resources used by the reader.
    /// </summary>
    ~jsonh_reader() noexcept {
        stream.reset();
    }

    /// <summary>
    /// Parses a single element from the reader.
    /// </summary>
    std::expected<json, std::string_view> parse_element() noexcept {
        std::stack<json> current_nodes;
        std::optional<std::string> current_property_name;

        auto submit_node = [&](json node) {
            // Root value
            if (current_nodes.empty()) {
                return true;
            }
            // Array item
            if (!current_property_name) {
                current_nodes.top().push_back(node);
                return false;
            }
            // Object property
            else {
                current_nodes.top()[current_property_name.value()] = node;
                current_property_name.reset();
                return false;
            }
        };
        auto start_node = [&](json node) {
            submit_node(node);
            current_nodes.push(node);
        };

        for (std::expected<jsonh_token, std::string>& token_result : read_element()) {
            // Check error
            if (!token_result) {
                return std::unexpected(token_result.error());
            }
            jsonh_token token = token_result.value();

            switch (token.json_type) {
                // Null
                case json_token_type::null: {
                    json node = json(nullptr);
                    if (submit_node(node)) {
                        return node;
                    }
                    break;
                }
                // True
                case json_token_type::true_bool: {
                    json node = json(true);
                    if (submit_node(node)) {
                        return node;
                    }
                    break;
                }
                // False
                case json_token_type::false_bool: {
                    json node = json(false);
                    if (submit_node(node)) {
                        return node;
                    }
                    break;
                }
                // String
                case json_token_type::string: {
                    json node = json(token.value);
                    if (submit_node(node)) {
                        return node;
                    }
                    break;
                }
                // Number
                case json_token_type::number: {
                    std::expected<long double, std::string_view> result = jsonh_number_parser::parse(token.value);
                    if (!result) {
                        return std::unexpected(result.error());
                    }
                    json node = json(result.value());
                    if (submit_node(node)) {
                        return node;
                    }
                    break;
                }
                // Start Object
                case json_token_type::start_object: {
                    json node = json::object();
                    start_node(node);
                    break;
                }
                // Start Array
                case json_token_type::start_array: {
                    json node = json::array();
                    start_node(node);
                    break;
                }
                // End Object/Array
                case json_token_type::end_object: case json_token_type::end_array: {
                    // Nested node
                    if (current_nodes.size() > 1) {
                        current_nodes.pop();
                    }
                    // Root node
                    else {
                        return current_nodes.top();
                    }
                    break;
                }
                // Property Name
                case json_token_type::property_name: {
                    current_property_name = token.value;
                    break;
                }
                // Comment
                case json_token_type::comment: {
                    break;
                }
                // Not implemented
                default: {
                    return std::unexpected("Token type not implemented");
                }
            }
        }

        // End of input
        return std::unexpected("Expected token, got end of input");
    }
    std::vector<std::expected<jsonh_token, std::string>> read_element() noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // Comments & whitespace
        for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Peek char
        std::optional<char> next = peek();
        if (!next) {
            tokens.push_back(std::unexpected("Expected token, got end of input"));
            return tokens;
        }

        // Object
        if (next.value() == '{') {
            for (std::expected<jsonh_token, std::string>& token : read_object()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }
        // Array
        else if (next.value() == '[') {
            for (std::expected<jsonh_token, std::string>& token : read_array()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }
        // Primitive value (null, true, false, string, number)
        else {
            std::expected<jsonh_token, std::string> token = read_primitive_element();
            if (!token) {
                tokens.push_back(std::unexpected(token.error()));
                return tokens;
            }

            // Detect braceless object from property name
            if (token.value().json_type == json_token_type::string) {
                // Try read property name
                std::vector<jsonh_token> property_name_tokens = {};
                for (std::expected<jsonh_token, std::string> property_name_token : read_property_name(token.value().value)) {
                    // Possible braceless object
                    if (property_name_token) {
                        property_name_tokens.push_back(property_name_token.value());
                    }
                    // Primitive value (error reading property name)
                    else {
                        tokens.push_back(token.value());
                        tokens.append_range(property_name_tokens);
                        return tokens;
                    }
                }
                // Braceless object
                for (std::expected<jsonh_token, std::string> object_token : read_braceless_object(property_name_tokens)) {
                    if (!object_token) {
                        tokens.push_back(object_token);
                        return tokens;
                    }
                    tokens.push_back(object_token);
                }
            }
            // Primitive value
            else {
                tokens.push_back(token);
            }
        }

        return tokens;
    }

private:
    const std::set<char32_t> reserved_chars = { '\\', ',', ':', '[', ']', '{', '}', '/', '#', '"', '\'' };
    const std::set<char32_t> newline_chars = { '\n', '\r', U'\u2028', U'\u2029' };
    const std::set<char32_t> whitespace_chars = {
        U'\u0020', U'\u00A0', U'\u1680', U'\u2000', U'\u2001', U'\u2002', U'\u2003', U'\u2004', U'\u2005',
        U'\u2006', U'\u2007', U'\u2008', U'\u2009', U'\u200A', U'\u202F', U'\u205F', U'\u3000', U'\u2028',
        U'\u2029', U'\u0009', U'\u000A', U'\u000B', U'\u000C', U'\u000D', U'\u0085',
    }; // https://learn.microsoft.com/en-us/dotnet/api/system.char.iswhitespace#remarks

    std::vector<std::expected<jsonh_token, std::string>> read_object() noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // Opening brace
        if (!read_one('{')) {
            // Braceless object
            for (std::expected<jsonh_token, std::string>& token : read_braceless_object()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
            return tokens;
        }
        // Start of object
        tokens.push_back(jsonh_token(json_token_type::start_object));

        while (true) {
            // Comments & whitespace
            for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }

            std::optional<char> next = peek();
            if (!next) {
                // End of incomplete object
                if (options.incomplete_inputs) {
                    tokens.push_back(jsonh_token(json_token_type::end_object));
                    return tokens;
                }
                // Missing closing brace
                tokens.push_back(std::unexpected("Expected `}` to end object, got end of input"));
                return tokens;
            }

            // Closing brace
            if (next == '}') {
                // End of object
                read();
                tokens.push_back(jsonh_token(json_token_type::end_object));
            }
            // Property
            else {
                for (std::expected<jsonh_token, std::string>& token : read_property()) {
                    if (!token) {
                        tokens.push_back(token);
                        return tokens;
                    }
                    tokens.push_back(token);
                }
            }
        }
    }
    std::vector<std::expected<jsonh_token, std::string>> read_braceless_object(std::optional<std::vector<jsonh_token>> property_name_tokens = std::nullopt) noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // Start of object
        tokens.push_back(jsonh_token(json_token_type::start_object));

        // Initial tokens
        if (property_name_tokens) {
            for (std::expected<jsonh_token, std::string>& token : read_property(property_name_tokens)) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }

        while (true) {
            // Comments & whitespace
            for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }

            if (!peek()) {
                // End of braceless object
                tokens.push_back(jsonh_token(json_token_type::end_object));
                return tokens;
            }

            // Property
            for (std::expected<jsonh_token, std::string>& token : read_property()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }
    }
    std::vector<std::expected<jsonh_token, std::string>> read_property(std::optional<std::vector<jsonh_token>> property_name_tokens = std::nullopt) noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // Property name
        if (property_name_tokens) {
            for (jsonh_token& token : property_name_tokens.value()) {
                tokens.push_back(token);
            }
        }
        else {
            for (std::expected<jsonh_token, std::string>& token : read_property_name()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }

        // Comments & whitespace
        for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Property value
        for (std::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Comments & whitespace
        for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Optional comma
        read_one(',');

        return tokens;
    }
    std::vector<std::expected<jsonh_token, std::string>> read_property_name(std::optional<std::string> string = std::nullopt) noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // String
        if (!string) {
            std::expected<jsonh_token, std::string> string_token = read_string();
            if (!string_token) {
                tokens.push_back(string_token);
                return tokens;
            }
            string = string_token.value().value;
        }

        // Comments & whitespace
        for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Colon
        if (!read_one(':')) {
            tokens.push_back(std::unexpected("Expected `:` after property name in object"));
            return tokens;
        }

        // End of property name
        tokens.push_back(jsonh_token(json_token_type::property_name, string.value()));

        return tokens;
    }
    std::vector<std::expected<jsonh_token, std::string>> read_array() noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // Opening bracket
        if (!read_one('[')) {
            tokens.push_back(std::unexpected("Expected `[` to start array"));
            return tokens;
        }
        // Start of array
        tokens.push_back(jsonh_token(json_token_type::start_object));

        while (true) {
            // Comments & whitespace
            for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }

            std::optional<char> next = peek();
            if (!next) {
                // End of incomplete array
                if (options.incomplete_inputs) {
                    tokens.push_back(jsonh_token(json_token_type::end_array));
                    return tokens;
                }
                // Missing closing bracket
                tokens.push_back(std::unexpected("Expected `]` to end array, got end of input"));
                return tokens;
            }

            // Closing bracket
            if (next == ']') {
                // End of array
                read();
                tokens.push_back(jsonh_token(json_token_type::end_array));
            }
            // Item
            else {
                for (std::expected<jsonh_token, std::string>& token : read_item()) {
                    if (!token) {
                        tokens.push_back(token);
                        return tokens;
                    }
                    tokens.push_back(token);
                }
            }
        }
    }
    std::vector<std::expected<jsonh_token, std::string>> read_item() noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // Element
        for (std::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Comments & whitespace
        for (std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Optional comma
        read_one(',');

        return tokens;
    }
    std::expected<jsonh_token, std::string> read_string() noexcept {
        // Start quote
        std::optional<char> start_quote = read_any({ '"', '\'' });
        if (!start_quote) {
            return read_quoteless_string();
        }

        // Count multiple start quotes
        size_t start_quote_counter = 1;
        while (read_one(start_quote.value())) {
            start_quote_counter++;
        }

        // Empty string
        if (start_quote_counter == 2) {
            return jsonh_token(json_token_type::string, "");
        }

        // Count multiple end quotes
        size_t end_quote_counter = 0;

        // Read string
        std::string string_builder;
        string_builder.reserve(64);

        while (true) {
            std::optional<char> next = read();
            if (!next) {
                return std::unexpected("Expected end of string, got end of input");
            }

            // Partial end quote was actually part of string
            if (next != start_quote) {
                string_builder.append(end_quote_counter, start_quote.value());
                end_quote_counter = 0;
            }

            // End quote
            if (next == start_quote) {
                end_quote_counter++;
                if (end_quote_counter == start_quote_counter) {
                    break;
                }
            }
            // Escape sequence
            else if (next == '\\') {
                std::expected<void, std::string> escape_sequence_result = read_escape_sequence(string_builder);
                if (!escape_sequence_result) {
                    return std::unexpected(escape_sequence_result.error());
                }
            }
            // Literal character
            else {
                string_builder += next.value();
            }
        }

        // Trim leading whitespace in multiline string
        if (start_quote_counter > 1) {
            // Count leading whitespace preceding closing quotes
            int last_newline_index = -1;
            for (int index = (int)(string_builder.length() - 1); index >= 0; index--) {
                if (newline_chars.contains(string_builder[index])) {
                    last_newline_index = index;
                }
            }
            if (last_newline_index != -1) {
                size_t leading_whitespace_count = string_builder.length() - last_newline_index;

                // Remove leading whitespace from each line
                if (leading_whitespace_count > 0) {
                    size_t current_leading_whitespace = 0;
                    bool is_leading_whitespace = true;

                    for (int index = 0; index < string_builder.length(); index++) {
                        char& next = string_builder[index];

                        // Newline
                        if (newline_chars.contains(next)) {
                            // Reset leading whitespace counter
                            current_leading_whitespace = 0;
                            // Enter leading whitespace
                            is_leading_whitespace = true;
                        }
                        // Leading whitespace
                        else if (is_leading_whitespace && current_leading_whitespace <= leading_whitespace_count) {
                            // Whitespace
                            if (whitespace_chars.contains(next)) {
                                // Increment leading whitespace counter
                                current_leading_whitespace++;
                                // Maximum leading whitespace reached
                                if (current_leading_whitespace == leading_whitespace_count) {
                                    // Remove leading whitespace
                                    string_builder.erase(index - current_leading_whitespace, current_leading_whitespace);
                                    // Exit leading whitespace
                                    is_leading_whitespace = false;
                                }
                            }
                            // Non-whitespace
                            else {
                                // Remove partial leading whitespace
                                string_builder.erase(index - current_leading_whitespace, current_leading_whitespace);
                                // Exit leading whitespace
                                is_leading_whitespace = false;
                            }
                        }
                    }

                    // Remove leading whitespace from last line
                    string_builder.erase(string_builder.length() - leading_whitespace_count, leading_whitespace_count);

                    // Remove leading newline
                    if (string_builder.length() >= 1) {
                        char& leading_char = string_builder[0];
                        if (newline_chars.contains(leading_char)) {
                            int newline_length = 1;
                            // Join CR LF
                            if (leading_char == '\r' && string_builder.length() >= 2 && string_builder[1] == '\n') {
                                newline_length = 2;
                            }

                            // Remove leading newline
                            string_builder.erase(0, newline_length);
                        }
                    }
                }
            }
        }

        // End of string
        return jsonh_token(json_token_type::string, string_builder);
    }
    std::expected<jsonh_token, std::string> read_quoteless_string(std::string initial_chars = "") noexcept {
        bool is_named_literal_possible = false;

        // Read quoteless string
        std::string string_builder = initial_chars;
        string_builder.reserve(64);

        while (true) {
            // Read char
            std::optional<char> next = peek();
            if (!next) {
                break;
            }

            // Read escape sequence
            if (next.value() == '\\') {
                read();
                std::expected<void, std::string> escape_sequence_result = read_escape_sequence(string_builder);
                if (!escape_sequence_result) {
                    return std::unexpected(escape_sequence_result.error());
                }
                is_named_literal_possible = false;
            }
            // End on reserved character
            else if (reserved_chars.contains(next.value())) {
                break;
            }
            // End on newline
            else if (newline_chars.contains(next.value())) {
                break;
            }
            // Append string char
            else {
                read();
                string_builder += next.value();
            }
        }

        // Ensure not empty
        if (string_builder.empty()) {
            return std::unexpected("Empty quoteless string");
        }

        // Trim trailing whitespace
        int trailing_whitespace_index = -1;
        for (int index = (int)(string_builder.length() - 1); index >= 0; index--) {
            if (whitespace_chars.contains(string_builder[index])) {
                trailing_whitespace_index = index;
            }
        }
        if (trailing_whitespace_index >= 0) {
            string_builder.erase(trailing_whitespace_index);
        }

        // Match named literal
        if (is_named_literal_possible) {
            if (string_builder == "null") {
                return jsonh_token(json_token_type::null);
            }
            else if (string_builder == "true") {
                return jsonh_token(json_token_type::true_bool);
            }
            else if (string_builder == "false") {
                return jsonh_token(json_token_type::false_bool);
            }
        }

        // End quoteless string
        return jsonh_token(json_token_type::string, string_builder);
    }
    bool detect_quoteless_string(std::string& whitespace_builder) {
        while (true) {
            // Read char
            std::optional<char> next = peek();
            if (!next) {
                break;
            }

            // Newline
            if (newline_chars.contains(next.value())) {
                // Quoteless strings cannot contain unescaped newlines
                return false;
            }

            // End of whitespace
            if (!whitespace_chars.contains(next.value())) {
                break;
            }

            // Whitespace
            whitespace_builder += next.value();
            read();
        }

        // Found quoteless string if found backslash or non-reserved char
        std::optional<char> next_char = peek();
        return next_char && (next_char.value() == '\\' || !reserved_chars.contains(next_char.value()));
    }
    std::expected<jsonh_token, std::string> read_number_or_quoteless_string() noexcept {
        // Read number
        std::string number_builder;
        number_builder.reserve(64);
        std::expected<jsonh_token, std::string> number = read_number(number_builder);
        if (!read_number(number_builder)) {
            // Try read quoteless string starting with number
            std::string whitespace_chars;
            whitespace_chars.reserve(64);
            if (detect_quoteless_string(whitespace_chars)) {
                return read_quoteless_string(number.value().value + whitespace_chars);
            }
            // Otherwise, accept number
            else {
                return number;
            }
        }
        // Read quoteless string starting with malformed number
        else {
            return read_quoteless_string(number_builder);
        }
    }
    std::expected<jsonh_token, std::string> read_number(std::string& number_builder) noexcept {
        // Read base
        std::string base_digits = "0123456789";
        if (read_one('0')) {
            number_builder += '0';

            std::optional<char> hex_base_char = read_any({ 'x', 'X' });
            if (hex_base_char) {
                number_builder += hex_base_char.value();
                base_digits = "0123456789ABCDEFabcdef";
            }
            else {
                std::optional<char> binary_base_char = read_any({ 'b', 'B' });
                if (hex_base_char) {
                    number_builder += binary_base_char.value();
                    base_digits = "01";
                }
                else {
                    std::optional<char> octal_base_char = read_any({ 'o', 'O' });
                    if (octal_base_char) {
                        number_builder += octal_base_char.value();
                        base_digits = "01234567";
                    }
                }
            }
        }

        // Read main number
        std::expected<void, std::string> main_result = read_number_no_exponent(number_builder, base_digits);
        if (!main_result) {
            return std::unexpected(main_result.error());
        }

        // Exponent
        std::optional<char> exponent_char = read_any({ 'e', 'E' });
        if (exponent_char) {
            number_builder += exponent_char.value();

            // Read exponent number
            std::expected<void, std::string> exponent_result = read_number_no_exponent(number_builder, base_digits);
            if (!exponent_result) {
                return std::unexpected(exponent_result.error());
            }
        }

        // End of number
        return jsonh_token(json_token_type::number, number_builder);
    }
    std::expected<void, std::string> read_number_no_exponent(std::string& number_builder, std::string_view base_digits) noexcept {
        // Read sign
        read_any({ '-', '+' });

        // Leading underscore
        if (read_one('_')) {
            return std::unexpected("Leading `_` in number");
        }

        bool is_fraction = false;

        while (true) {
            // Peek char
            std::optional<char> next = peek();
            if (!next) {
                break;
            }

            // Digit
            if (base_digits.contains(next.value())) {
                read();
                number_builder += next.value();
            }
            // Decimal point
            else if (next.value() == '.') {
                read();
                number_builder += next.value();

                // Duplicate decimal point
                if (is_fraction) {
                    return std::unexpected("Duplicate `.` in number");
                }
                is_fraction = true;
            }
            // Underscore
            else if (next.value() == '_') {
                read();
                number_builder += next.value();
            }
            // Other
            else {
                break;
            }
        }

        // Ensure not empty
        if (number_builder.empty()) {
            return std::unexpected("Empty number");
        }

        // Trailing underscore
        if (number_builder.ends_with('_')) {
            return std::unexpected("Trailing `_` in number");
        }

        // End of number
        return std::expected<void, std::string>(); // Success
    }
    std::expected<jsonh_token, std::string> read_primitive_element() noexcept {
        // Peek char
        std::optional<char> next = peek();
        if (!next) {
            return std::unexpected("Expected primitive element, got end of input");
        }

        // Number
        if ((next >= '0' && next <= '9') || (next == '-' || next == '+') || next == '.') {
            return read_number_or_quoteless_string();
        }
        // String
        else if (next == '"' || next == '\'') {
            return read_string();
        }
        // Quoteless string (or named literal)
        else {
            return read_quoteless_string();
        }
    }
    std::vector<std::expected<jsonh_token, std::string>> read_comments_and_whitespace() noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        while (true) {
            // Whitespace
            read_whitespace();

            // Peek char
            std::optional<char> next = peek();

            // Comment
            if (next == '#' || next == '/') {
                tokens.push_back(read_comment());
            }
            // End of comments
            else {
                break;
            }
        }

        return tokens;
    }
    std::expected<jsonh_token, std::string> read_comment() noexcept {
        bool block_comment = false;

        // Hash-styled comment
        if (read_one('#')) {
        }
        else if (read_one('/')) {
            // Line-styled comment
            if (read_one('/')) {
            }
            // Block-styled comment
            else if (read_one('*')) {
                block_comment = true;
            }
            else {
                return std::unexpected("Unexpected '/'");
            }
        }
        else {
            return std::unexpected("Unexpected character");
        }

        // Read comment
        std::string comment_builder;
        comment_builder.reserve(64);

        while (true) {
            // Read char
            std::optional<char> next = read();

            if (block_comment) {
                // Error
                if (!next) {
                    return std::unexpected("Expected end of block comment, got end of input");
                }
                // End of block comment
                if (next == '*' && read_one('/')) {
                    return jsonh_token(json_token_type::comment, comment_builder);
                }
            }
            else {
                // End of line comment
                if (!next || newline_chars.contains(next.value())) {
                    return jsonh_token(json_token_type::comment, comment_builder);
                }
            }

            // Comment char
            comment_builder += next.value();
        }
    }
    void read_whitespace() noexcept {
        while (true) {
            // Peek char
            std::optional<char> next = peek();
            if (!next) {
                return;
            }

            // Whitespace
            if (whitespace_chars.contains(*next)) {
                read();
            }
            // End of whitespace
            else {
                return;
            }
        }
    }
    std::expected<unsigned int, std::string> read_hex_sequence(int length) noexcept {
        std::string hex_chars(length, '\0');

        for (int index = 0; index < length; index++) {
            std::optional<char> next = read();

            // Hex digit
            if ((next >= '0' && next <= '9') || (next >= 'A' && next <= 'F') || (next >= 'a' && next <= 'f')) {
                hex_chars[index] = next.value();
            }
            // Unexpected char
            else {
                return std::unexpected("Incorrect number of hexadecimal digits in unicode escape sequence");
            }
        }

        // Parse unicode character from hex digits
        return (unsigned int)std::stoul(hex_chars, nullptr, 16);
    }
    std::expected<void, std::string> read_escape_sequence(std::string& string_builder) noexcept {
        std::optional<char> escape_char = read();
        if (!escape_char) {
            return std::unexpected("Expected escape sequence, got end of input");
        }

        // Reverse solidus
        if (escape_char.value() == '\\') {
            string_builder += '\\';
        }
        // Backspace
        else if (escape_char.value() == 'b') {
            string_builder += '\b';
        }
        // Form feed
        else if (escape_char.value() == 'f') {
            string_builder += '\f';
        }
        // Newline
        else if (escape_char.value() == 'n') {
            string_builder += '\n';
        }
        // Carriage return
        else if (escape_char.value() == 'r') {
            string_builder += '\r';
        }
        // Tab
        else if (escape_char.value() == 't') {
            string_builder += '\t';
        }
        // Vertical tab
        else if (escape_char.value() == 'v') {
            string_builder += '\v';
        }
        // Null
        else if (escape_char.value() == '0') {
            string_builder += '\0';
        }
        // Alert
        else if (escape_char.value() == 'a') {
            string_builder += '\a';
        }
        // Escape
        else if (escape_char.value() == 'e') {
            string_builder += '\u001b';
        }
        // Unicode hex sequence
        else if (escape_char.value() == 'u') {
            std::expected<unsigned int, std::string> hex_code = read_hex_sequence(4);
            if (!hex_code) {
                return std::unexpected(hex_code.error());
            }
            char16_t utf16_char = (char16_t)hex_code.value();
            string_builder += convert_utf16_to_utf8(utf16_char);
        }
        // Short unicode hex sequence
        else if (escape_char.value() == 'x') {
            std::expected<unsigned int, std::string> hex_code = read_hex_sequence(2);
            if (!hex_code) {
                return std::unexpected(hex_code.error());
            }
            char16_t utf16_char = (char16_t)hex_code.value();
            string_builder += convert_utf16_to_utf8(utf16_char);
        }
        // Long unicode hex sequence
        else if (escape_char.value() == 'U') {
            std::expected<unsigned int, std::string> hex_code = read_hex_sequence(8);
            if (!hex_code) {
                return std::unexpected(hex_code.error());
            }
            char32_t utf32_char = (char32_t)hex_code.value();
            string_builder += convert_utf32_to_utf8(utf32_char);
        }
        // Escaped newline
        else if (newline_chars.contains(escape_char.value())) {
            // Join CR LF
            if (escape_char == '\r') {
                read_one('\n');
            }
        }
        // Other
        else {
            string_builder += escape_char.value();
        }
        return std::expected<void, std::string>(); // Success
    }
    std::optional<char> peek() const noexcept {
        int next_int = stream->peek();
        if (next_int < 0) {
            return {};
        }
        char next = (char)next_int;
        return next;
    }
    std::optional<char> read() noexcept {
        int next_int = stream->get();
        if (next_int < 0) {
            return {};
        }
        char next = (char)next_int;
        char_counter++;
        return next;
    }
    bool read_one(char option) noexcept {
        if (peek() == option) {
            read();
            return true;
        }
        return false;
    }
    std::optional<char> read_any(const std::set<char>& options) noexcept {
        // Peek char
        std::optional<char> next = peek();
        if (!next) {
            return {};
        }
        // Match option
        if (!options.contains(next.value())) {
            return {};
        }
        // Option matched
        read();
        return next;
    }
    static std::string convert_utf16_to_utf8(const char16_t utf16_char) noexcept {
        std::u16string utf16_string = { utf16_char };
        std::string utf8_result = utf8::utf16to8(utf16_string);
        return utf8_result;
    }
    static std::string convert_utf32_to_utf8(const char32_t utf32_char) noexcept {
        std::u32string utf32_string = { utf32_char };
        std::string utf8_result = utf8::utf32to8(utf32_string);
        return utf8_result;
    }
};

}