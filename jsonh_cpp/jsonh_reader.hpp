#pragma once

#include <string> // for std::string
#include <sstream> // for std::istringstream
#include <vector> // for std::vector
#include <set> // for std::set
#include <stack> // for std::stack
#include <optional> // for std::optional
#include <expected> // for std::expected
#include "nlohmann/json.hpp" // for nlohmann::json
#include "jsonh_token.hpp" // for jsonh::jsonh_token
#include "jsonh_reader_options.hpp" // for jsonh::jsonh_reader_options
#include "jsonh_number_parser.hpp" // for jsonh::jsonh_number_parser
#include "utf8_reader.hpp" // for jsonh::utf8_reader

using namespace nlohmann;

namespace jsonh {

/// <summary>
/// A reader that reads tokens from a UTF-8 input stream.
/// </summary>
class jsonh_reader : utf8_reader {
public:
    /// <summary>
    /// The options to use when reading JSONH.
    /// </summary>
    jsonh_reader_options options;

    /// <summary>
    /// Constructs a reader that reads JSONH from a UTF-8 stream.
    /// </summary>
    jsonh_reader(std::unique_ptr<std::istream> stream, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : utf8_reader(std::move(stream)) {
        this->options = options;
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a UTF-8 stream.
    /// </summary>
    jsonh_reader(std::istream& stream, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader(std::unique_ptr<std::istream>(&stream)) {
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a UTF-8 string.
    /// </summary>
    jsonh_reader(const std::string& string, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader(std::make_unique<std::istringstream>(string), options) {
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a UTF-8 string_view converted to a string.
    /// </summary>
    jsonh_reader(const std::string_view& string_view, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader(std::string(string_view), options) {
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a UTF-8 char pointer converted to a string.
    /// </summary>
    jsonh_reader(const char* string, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader(std::string(string), options) {
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a UTF-8 string converted to a string.
    /// </summary>
    jsonh_reader(const std::u8string& string, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader((const char*)string.data(), options) {
    }

    /// <summary>
    /// Parses a single element from a UTF-8 stream and deserializes it as <c>t</c>.
    /// </summary>
    template <typename t>
    static std::expected<t, std::string> parse_element(std::istream& stream) noexcept {
        std::expected<json, std::string> node = parse_element(stream);
        if (!node) {
            return std::unexpected(node.error());
        }
        return node.value().template get<t>();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 stream.
    /// </summary>
    static std::expected<json, std::string> parse_element(std::istream& stream) noexcept {
        return jsonh_reader(stream).parse_element();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 string and deserializes it as <c>t</c>.
    /// </summary>
    template <typename t>
    static std::expected<t, std::string> parse_element(const std::string& string) noexcept {
        std::expected<json, std::string> node = parse_element(string);
        if (!node) {
            return std::unexpected(node.error());
        }
        return node.value().template get<t>();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 string.
    /// </summary>
    static std::expected<json, std::string> parse_element(const std::string& string) noexcept {
        return jsonh_reader(string).parse_element();
    }

    /// <summary>
    /// Parses a single element from the reader and deserializes it as <c>t</c>.
    /// </summary>
    template <typename t>
    std::expected<t, std::string> parse_element() noexcept {
        std::expected<json, std::string> node = parse_element();
        if (!node) {
            return std::unexpected(node.error());
        }
        return node.value().template get<t>();
    }
    /// <summary>
    /// Parses a single element from the reader.
    /// </summary>
    std::expected<json, std::string> parse_element() noexcept {
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

        for (const std::expected<jsonh_token, std::string>& token_result : read_element()) {
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
                    std::expected<long double, std::string> result = jsonh_number_parser::parse(token.value);
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
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Peek rune
        std::optional<std::string> next = peek();
        if (!next) {
            tokens.push_back(std::unexpected("Expected token, got end of input"));
            return tokens;
        }

        // Object
        if (next.value() == "{") {
            for (const std::expected<jsonh_token, std::string>& token : read_object()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }
        // Array
        else if (next.value() == "[") {
            for (const std::expected<jsonh_token, std::string>& token : read_array()) {
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
                for (const std::expected<jsonh_token, std::string>& property_name_token : read_property_name(token.value().value)) {
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
                for (const std::expected<jsonh_token, std::string> object_token : read_braceless_object(property_name_tokens)) {
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
    const std::set<std::string> reserved_runes = { "\\", ",", ":", "[", "]", "{", "}", "/", "#", "\"", "'" };
    const std::set<std::string> newline_runes = { "\n", "\r", "\u2028", "\u2029" };
    const std::set<std::string> whitespace_runes = {
        "\u0020", "\u00A0", "\u1680", "\u2000", "\u2001", "\u2002", "\u2003", "\u2004", "\u2005",
        "\u2006", "\u2007", "\u2008", "\u2009", "\u200A", "\u202F", "\u205F", "\u3000", "\u2028",
        "\u2029", "\u0009", "\u000A", "\u000B", "\u000C", "\u000D", "\u0085",
    }; // https://learn.microsoft.com/en-us/dotnet/api/system.char.iswhitespace#remarks

    std::vector<std::expected<jsonh_token, std::string>> read_object() noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // Opening brace
        if (!read_one("{")) {
            // Braceless object
            for (const std::expected<jsonh_token, std::string>& token : read_braceless_object()) {
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
            for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }

            std::optional<std::string> next = peek();
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
            if (next == "}") {
                // End of object
                read();
                tokens.push_back(jsonh_token(json_token_type::end_object));
            }
            // Property
            else {
                for (const std::expected<jsonh_token, std::string>& token : read_property()) {
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
            for (const std::expected<jsonh_token, std::string>& token : read_property(property_name_tokens)) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }

        while (true) {
            // Comments & whitespace
            for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
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
            for (const std::expected<jsonh_token, std::string>& token : read_property()) {
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
            for (const jsonh_token& token : property_name_tokens.value()) {
                tokens.push_back(token);
            }
        }
        else {
            for (const std::expected<jsonh_token, std::string>& token : read_property_name()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }

        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Property value
        for (const std::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Optional comma
        read_one(",");

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
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Colon
        if (!read_one(":")) {
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
        if (!read_one("[")) {
            tokens.push_back(std::unexpected("Expected `[` to start array"));
            return tokens;
        }
        // Start of array
        tokens.push_back(jsonh_token(json_token_type::start_array));

        while (true) {
            // Comments & whitespace
            for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }

            std::optional<std::string> next = peek();
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
            if (next == "]") {
                // End of array
                read();
                tokens.push_back(jsonh_token(json_token_type::end_array));
            }
            // Item
            else {
                for (const std::expected<jsonh_token, std::string>& token : read_item()) {
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
        for (const std::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Optional comma
        read_one(",");

        return tokens;
    }
    std::expected<jsonh_token, std::string> read_string() noexcept {
        // Start quote
        std::optional<std::string> start_quote = read_any({ "\"", "'" });
        if (!start_quote) {
            return read_quoteless_string();
        }
        char start_quote_char = start_quote.value()[0];

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
        // Find last newline
        size_t last_newline_index = -1;

        // Read string
        std::string string_builder;
        string_builder.reserve(64);

        while (true) {
            std::optional<std::string> next = read();
            if (!next) {
                return std::unexpected("Expected end of string, got end of input");
            }

            // Partial end quote was actually part of string
            if (next != start_quote) {
                string_builder.append(end_quote_counter, start_quote_char);
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
            else if (next == "\\") {
                std::expected<void, std::string> escape_sequence_result = read_escape_sequence(string_builder);
                if (!escape_sequence_result) {
                    return std::unexpected(escape_sequence_result.error());
                }
            }
            // Literal character
            else {
                // Newline
                if (newline_runes.contains(next.value())) {
                    last_newline_index = string_builder.size();
                }

                string_builder += next.value();
            }
        }

        // Condition: skip remaining steps unless started with multiple quotes
        if (start_quote_counter > 1) {
            // Pass 1: count leading whitespace -> newline
            utf8_reader string_builder_reader1(string_builder);
            bool has_leading_whitespace_newline = false;
            size_t leading_whitespace_newline_counter = 0;
            while (true) {
                std::optional<std::string> next = string_builder_reader1.read();
                if (!next) {
                    break;
                }
                size_t index = string_builder_reader1.position();

                // Newline
                if (newline_runes.contains(next.value())) {
                    // Join CR LF
                    if (next.value() == "\r" && peek() == "\n") {
                        string_builder_reader1.read();
                        index = string_builder_reader1.position();
                    }

                    has_leading_whitespace_newline = true;
                    leading_whitespace_newline_counter = index + 1;
                    break;
                }
                // Non-whitespace
                else if (!whitespace_runes.contains(next.value())) {
                    break;
                }
            }

            // Condition: skip remaining steps if pass 1 failed
            if (has_leading_whitespace_newline) {
                // Pass 2: count trailing newline -> whitespace
                utf8_reader string_builder_reader2(string_builder);
                bool has_trailing_newline_whitespace = false;
                size_t last_newline_index = 0;
                int trailing_whitespace_counter = 0;
                while (true) {
                    std::optional<std::string> next = string_builder_reader2.read();
                    if (!next) {
                        break;
                    }
                    size_t index = string_builder_reader2.position();

                    // Newline
                    if (newline_runes.contains(next.value())) {
                        has_trailing_newline_whitespace = true;
                        last_newline_index = index;
                        trailing_whitespace_counter = 0;

                        // Join CR LF
                        if (next.value() == "\r" && peek() == "\n") {
                            string_builder_reader2.read();
                            index = string_builder_reader2.position();
                        }
                    }
                    // Whitespace
                    else if (whitespace_runes.contains(next.value())) {
                        trailing_whitespace_counter++;
                    }
                    // Non-whitespace
                    else {
                        has_trailing_newline_whitespace = false;
                        trailing_whitespace_counter = 0;
                    }
                }

                // Condition: skip remaining steps if pass 2 failed
                if (has_trailing_newline_whitespace) {
                    // Pass 3: strip last newline -> whitespace
                    string_builder.erase(last_newline_index, string_builder.size() - last_newline_index);

                    // Pass 4: strip first whitespace -> newline
                    string_builder.erase(0, leading_whitespace_newline_counter);

                    // Condition: skip remaining steps if no trailing whitespace
                    if (trailing_whitespace_counter > 0) {
                        // Pass 5: strip line-leading whitespace
                        utf8_reader string_builder_reader3(string_builder);
                        bool is_line_leading_whitespace = true;
                        int line_leading_whitespace_counter = 0;
                        while (true) {
                            std::optional<std::string> next = string_builder_reader3.read();
                            if (!next) {
                                break;
                            }
                            size_t index = string_builder_reader3.position();

                            // Newline
                            if (newline_runes.contains(next.value())) {
                                is_line_leading_whitespace = true;
                                line_leading_whitespace_counter = 0;
                            }
                            // Whitespace
                            else if (whitespace_runes.contains(next.value())) {
                                if (is_line_leading_whitespace) {
                                    // Increment line-leading whitespace
                                    line_leading_whitespace_counter++;

                                    // Maximum line-leading whitespace reached
                                    if (line_leading_whitespace_counter == trailing_whitespace_counter) {
                                        // Remove line-leading whitespace
                                        string_builder.erase(index + 1 - line_leading_whitespace_counter, line_leading_whitespace_counter);
                                        index -= line_leading_whitespace_counter;
                                        // Exit line-leading whitespace
                                        is_line_leading_whitespace = false;
                                    }
                                }
                            }
                            // Non-whitespace
                            else {
                                if (is_line_leading_whitespace) {
                                    // Remove partial line-leading whitespace
                                    string_builder.erase(index - line_leading_whitespace_counter, line_leading_whitespace_counter);
                                    index -= line_leading_whitespace_counter;
                                    // Exit line-leading whitespace
                                    is_line_leading_whitespace = false;
                                }
                            }
                        }
                    }
                }
            }
        }

        // End of string
        return jsonh_token(json_token_type::string, string_builder);
    }
    std::expected<jsonh_token, std::string> read_quoteless_string(std::string initial_chars = "") noexcept {
        bool is_named_literal_possible = true;

        // Read quoteless string
        std::string string_builder = initial_chars;
        string_builder.reserve(64);

        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Escape sequence
            if (next.value() == "\\") {
                read();
                std::expected<void, std::string> escape_sequence_result = read_escape_sequence(string_builder);
                if (!escape_sequence_result) {
                    return std::unexpected(escape_sequence_result.error());
                }
                is_named_literal_possible = false;
            }
            // End on reserved character
            else if (reserved_runes.contains(next.value())) {
                break;
            }
            // End on newline
            else if (newline_runes.contains(next.value())) {
                break;
            }
            // Literal character
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
        utf8_reader string_builder_reader(string_builder);
        string_builder_reader.seek(0, std::ios_base::end);
        while (true) {
            size_t original_position = string_builder_reader.position();
            std::optional<std::string> next = string_builder_reader.read_reverse();

            // Non-whitespace
            if (!next || !whitespace_runes.contains(next.value())) {
                string_builder.erase(original_position);
                break;
            }
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
            // Read rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Newline
            if (newline_runes.contains(next.value())) {
                // Quoteless strings cannot contain unescaped newlines
                return false;
            }

            // End of whitespace
            if (!whitespace_runes.contains(next.value())) {
                break;
            }

            // Whitespace
            whitespace_builder += next.value();
            read();
        }

        // Found quoteless string if found backslash or non-reserved char
        std::optional<std::string> next_char = peek();
        return next_char && (next_char.value() == "\\" || !reserved_runes.contains(next_char.value()));
    }
    std::expected<jsonh_token, std::string> read_number_or_quoteless_string() noexcept {
        // Read number
        std::string number_builder;
        number_builder.reserve(64);
        std::expected<jsonh_token, std::string> number = read_number(number_builder);
        if (read_number(number_builder)) {
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
        if (read_one("0")) {
            number_builder += '0';

            std::optional<std::string> hex_base_char = read_any({ "x", "X" });
            if (hex_base_char) {
                number_builder += hex_base_char.value();
                base_digits = "0123456789ABCDEFabcdef";
            }
            else {
                std::optional<std::string> binary_base_char = read_any({ "b", "B" });
                if (hex_base_char) {
                    number_builder += binary_base_char.value();
                    base_digits = "01";
                }
                else {
                    std::optional<std::string> octal_base_char = read_any({ "o", "O" });
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
        std::optional<std::string> exponent_char = read_any({ "e", "E" });
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
        read_any({ "-", "+" });

        // Leading underscore
        if (read_one("_")) {
            return std::unexpected("Leading `_` in number");
        }

        bool is_fraction = false;

        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Digit
            if (base_digits.contains(next.value())) {
                read();
                number_builder += next.value();
            }
            // Decimal point
            else if (next.value() == ".") {
                read();
                number_builder += next.value();

                // Duplicate decimal point
                if (is_fraction) {
                    return std::unexpected("Duplicate `.` in number");
                }
                is_fraction = true;
            }
            // Underscore
            else if (next.value() == "_") {
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
        // Peek rune
        std::optional<std::string> next = peek();
        if (!next) {
            return std::unexpected("Expected primitive element, got end of input");
        }

        // Number
        if ((next >= "0" && next <= "9") || (next == "-" || next == "+") || next == ".") {
            return read_number_or_quoteless_string();
        }
        // String
        else if (next == "\"" || next == "'") {
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

            // Peek rune
            std::optional<std::string> next = peek();

            // Comment
            if (next == "#" || next == "/") {
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
        if (read_one("#")) {
        }
        else if (read_one("/")) {
            // Line-styled comment
            if (read_one("/")) {
            }
            // Block-styled comment
            else if (read_one("*")) {
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
            // Read rune
            std::optional<std::string> next = read();

            if (block_comment) {
                // Error
                if (!next) {
                    return std::unexpected("Expected end of block comment, got end of input");
                }
                // End of block comment
                if (next == "*" && read_one("/")) {
                    return jsonh_token(json_token_type::comment, comment_builder);
                }
            }
            else {
                // End of line comment
                if (!next || newline_runes.contains(next.value())) {
                    return jsonh_token(json_token_type::comment, comment_builder);
                }
            }

            // Comment char
            comment_builder += next.value();
        }
    }
    void read_whitespace() noexcept {
        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                return;
            }

            // Whitespace
            if (whitespace_runes.contains(next.value())) {
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
            std::optional<std::string> next = read();

            // Hex digit
            if ((next >= "0" && next <= "9") || (next >= "A" && next <= "F") || (next >= "a" && next <= "f")) {
                char next_char = next.value()[0];
                hex_chars[index] = next_char;
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
        std::optional<std::string> escape_char = read();
        if (!escape_char) {
            return std::unexpected("Expected escape sequence, got end of input");
        }

        // Reverse solidus
        if (escape_char.value() == "\\") {
            string_builder += '\\';
        }
        // Backspace
        else if (escape_char.value() == "b") {
            string_builder += '\b';
        }
        // Form feed
        else if (escape_char.value() == "f") {
            string_builder += '\f';
        }
        // Newline
        else if (escape_char.value() == "n") {
            string_builder += '\n';
        }
        // Carriage return
        else if (escape_char.value() == "r") {
            string_builder += '\r';
        }
        // Tab
        else if (escape_char.value() == "t") {
            string_builder += '\t';
        }
        // Vertical tab
        else if (escape_char.value() == "v") {
            string_builder += '\v';
        }
        // Null
        else if (escape_char.value() == "0") {
            string_builder += '\0';
        }
        // Alert
        else if (escape_char.value() == "a") {
            string_builder += '\a';
        }
        // Escape
        else if (escape_char.value() == "e") {
            string_builder += '\u001b';
        }
        // Unicode hex sequence
        else if (escape_char.value() == "u") {
            std::expected<unsigned int, std::string> hex_code = read_hex_sequence(4);
            if (!hex_code) {
                return std::unexpected(hex_code.error());
            }
            std::expected<std::string, std::string> hex_rune = codepoint_to_utf8(hex_code.value());
            if (!hex_rune) {
                return std::unexpected(hex_rune.error());
            }
            string_builder += hex_rune.value();
        }
        // Short unicode hex sequence
        else if (escape_char.value() == "x") {
            std::expected<unsigned int, std::string> hex_code = read_hex_sequence(2);
            if (!hex_code) {
                return std::unexpected(hex_code.error());
            }
            std::expected<std::string, std::string> hex_rune = codepoint_to_utf8(hex_code.value());
            if (!hex_rune) {
                return std::unexpected(hex_rune.error());
            }
            string_builder += hex_rune.value();
        }
        // Long unicode hex sequence
        else if (escape_char.value() == "U") {
            std::expected<unsigned int, std::string> hex_code = read_hex_sequence(8);
            if (!hex_code) {
                return std::unexpected(hex_code.error());
            }
            std::expected<std::string, std::string> hex_rune = codepoint_to_utf8(hex_code.value());
            if (!hex_rune) {
                return std::unexpected(hex_rune.error());
            }
            string_builder += hex_rune.value();
        }
        // Escaped newline
        else if (newline_runes.contains(escape_char.value())) {
            // Join CR LF
            if (escape_char == "\r") {
                read_one("\n");
            }
        }
        // Other
        else {
            string_builder += escape_char.value();
        }
        return std::expected<void, std::string>(); // Success
    }
    static std::expected<std::string, std::string> codepoint_to_utf8(unsigned int code_point) {
        std::string result;
        // 1-byte UTF-8
        if (code_point <= 0x7F) {
            result += (char)code_point;
        }
        // 2-byte UTF-8
        else if (code_point <= 0x7FF) {
            result += (char)(0xC0 | (code_point >> 6));
            result += (char)(0x80 | (code_point & 0x3F));
        }
        // 3-byte UTF-8
        else if (code_point <= 0xFFFF) {
            result += (char)(0xE0 | (code_point >> 12));
            result += (char)(0x80 | ((code_point >> 6) & 0x3F));
            result += (char)(0x80 | (code_point & 0x3F));
        }
        // 4-byte UTF-8
        else if (code_point <= 0x10FFFF) {
            result += (char)(0xF0 | (code_point >> 18));
            result += (char)(0x80 | ((code_point >> 12) & 0x3F));
            result += (char)(0x80 | ((code_point >> 6) & 0x3F));
            result += (char)(0x80 | (code_point & 0x3F));
        }
        // Invalid UTF-8
        else {
            return std::unexpected("Invalid Unicode code point");
        }
        return result;
    }
};

}