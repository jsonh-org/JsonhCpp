#pragma once

#include <string> // for std::string
#include <sstream> // for std::istringstream
#include <vector> // for std::vector
#include <set> // for std::set
#include <stack> // for std::stack
#include <optional> // for std::optional
#include "martinmoene/expected.hpp" // for nonstd::expected (std::expected backport)
#include "nlohmann/json.hpp" // for nlohmann::json
#include "jsonh_token.hpp" // for jsonh_cpp::jsonh_token
#include "jsonh_reader_options.hpp" // for jsonh_cpp::jsonh_reader_options
#include "jsonh_number_parser.hpp" // for jsonh_cpp::jsonh_number_parser
#include "utf8_reader.hpp" // for jsonh_cpp::utf8_reader

using namespace nlohmann;

namespace jsonh_cpp {

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
    /// Constructs a reader that reads JSONH from a UTF-8 input stream.
    /// </summary>
    jsonh_reader(std::unique_ptr<std::istream> stream, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : utf8_reader(std::move(stream)) {
        this->options = options;
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a UTF-8 input stream.
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
    /// Parses a single element from a UTF-8 input stream and deserializes it as <typeparamref name="T"/>.
    /// </summary>
    template <typename T>
    static nonstd::expected<T, std::string> parse_element(std::unique_ptr<std::istream> stream) noexcept {
        return jsonh_reader(std::move(stream)).parse_element<T>();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 input stream.
    /// </summary>
    static nonstd::expected<json, std::string> parse_element(std::unique_ptr<std::istream> stream) noexcept {
        return jsonh_reader(std::move(stream)).parse_element();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 input stream and deserializes it as <typeparamref name="T"/>.
    /// </summary>
    template <typename T>
    static nonstd::expected<T, std::string> parse_element(std::istream& stream) noexcept {
        return jsonh_reader(stream).parse_element<T>();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 input stream.
    /// </summary>
    static nonstd::expected<json, std::string> parse_element(std::istream& stream) noexcept {
        return jsonh_reader(stream).parse_element();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 string and deserializes it as <typeparamref name="T"/>.
    /// </summary>
    template <typename T>
    static nonstd::expected<T, std::string> parse_element(const std::string& string) noexcept {
        return jsonh_reader(string).parse_element<T>();
    }
    /// <summary>
    /// Parses a single element from a UTF-8 string.
    /// </summary>
    static nonstd::expected<json, std::string> parse_element(const std::string& string) noexcept {
        return jsonh_reader(string).parse_element();
    }

    /// <summary>
    /// Parses a single element from the reader and deserializes it as <typeparamref name="T"/>.
    /// </summary>
    template <typename T>
    nonstd::expected<T, std::string> parse_element() noexcept {
        nonstd::expected<json, std::string> node = parse_element();
        if (!node) {
            return nonstd::unexpected<std::string>(node.error());
        }
        return node.value().template get<T>();
    }
    /// <summary>
    /// Parses a single element from the reader.
    /// </summary>
    nonstd::expected<json, std::string> parse_element() noexcept {
        std::stack<json> current_nodes;
        std::optional<std::string> current_property_name;

        auto submit_node = [&](const json& node) {
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
        auto start_node = [&](const json& node) {
            submit_node(node);
            current_nodes.push(node);
        };

        for (const nonstd::expected<jsonh_token, std::string>& token_result : read_element()) {
            // Check error
            if (!token_result) {
                return nonstd::unexpected<std::string>(token_result.error());
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
                    nonstd::expected<long double, std::string> result = jsonh_number_parser::parse(token.value);
                    if (!result) {
                        return nonstd::unexpected<std::string>(result.error());
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
                    return nonstd::unexpected<std::string>("Token type not implemented");
                }
            }
        }

        // End of input
        return nonstd::unexpected<std::string>("Expected token, got end of input");
    }
    /// <summary>
    /// Tries to find the given property name in the reader.<br/>
    /// For example, to find <c>c</c>:
    /// <code>
    /// // Original position
    /// {
    ///   "a": "1",
    ///   "b": {
    ///     "c": "2"
    ///   },
    ///   "c":/* Final position */ "3"
    /// }
    /// </code>
    /// </summary>
    bool find_property_value(const std::string& property_name) noexcept {
        long long current_depth = 0;

        for (const nonstd::expected<jsonh_token, std::string>& token_result : read_element()) {
            // Check error
            if (!token_result) {
                return false;
            }

            switch (token_result.value().json_type) {
                // Start structure
                case json_token_type::start_object: case json_token_type::start_array: {
                    current_depth++;
                    break;
                }
                // End structure
                case json_token_type::end_object: case json_token_type::end_array: {
                    current_depth--;
                    break;
                }
                // Property name
                case json_token_type::property_name: {
                    if (current_depth == 1 && token_result.value().value == property_name) {
                        // Path found
                        return true;
                    }
                    break;
                }
                // Other
                default: {
                    break;
                }
            }
        }

        // Path not found
        return false;
    }
    /// <summary>
    /// Reads a single element from the reader.
    /// </summary>
    std::vector<nonstd::expected<jsonh_token, std::string>> read_element() noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        // Comments & whitespace
        for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Peek rune
        std::optional<std::string> next = peek();
        if (!next) {
            tokens.push_back(nonstd::unexpected<std::string>("Expected token, got end of input"));
            return tokens;
        }

        // Object
        if (next.value() == "{") {
            for (const nonstd::expected<jsonh_token, std::string>& token : read_object()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }
        // Array
        else if (next.value() == "[") {
            for (const nonstd::expected<jsonh_token, std::string>& token : read_array()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }
        // Primitive value (null, true, false, string, number)
        else {
            nonstd::expected<jsonh_token, std::string> token = read_primitive_element();
            if (!token) {
                tokens.push_back(nonstd::unexpected<std::string>(token.error()));
                return tokens;
            }

            // Detect braceless object from property name
            if (token.value().json_type == json_token_type::string) {
                // Try read property name
                std::vector<jsonh_token> property_name_tokens = {};
                for (const nonstd::expected<jsonh_token, std::string>& property_name_token : read_property_name(token.value().value)) {
                    // Possible braceless object
                    if (property_name_token) {
                        property_name_tokens.push_back(property_name_token.value());
                    }
                    // Primitive value (error reading property name)
                    else {
                        tokens.push_back(token.value());
                        for (const jsonh_token& property_name_token : property_name_tokens) {
                            property_name_tokens.push_back(property_name_token);
                        }
                        return tokens;
                    }
                }
                // Braceless object
                for (const nonstd::expected<jsonh_token, std::string> object_token : read_braceless_object(property_name_tokens)) {
                    if (!object_token) {
                        tokens.push_back(object_token);
                        return tokens;
                    }
                    tokens.push_back(object_token);
                }
            }
            // Primitive value
            else {
                tokens.push_back(token.value());
            }
        }

        return tokens;
    }

private:
    /// <summary>
    /// Runes that cannot be used unescaped in quoteless strings.
    /// </summary>
    const std::set<std::string> reserved_runes = { "\\", ",", ":", "[", "]", "{", "}", "/", "#", "\"", "'" };
    /// <summary>
    /// Runes that are considered newlines.
    /// </summary>
    const std::set<std::string> newline_runes = { "\n", "\r", "\u2028", "\u2029" };
    /// <summary>
    /// Runes that are considered whitespace.
    /// </summary>
    const std::set<std::string> whitespace_runes = {
        "\u0020", "\u00A0", "\u1680", "\u2000", "\u2001", "\u2002", "\u2003", "\u2004", "\u2005",
        "\u2006", "\u2007", "\u2008", "\u2009", "\u200A", "\u202F", "\u205F", "\u3000", "\u2028",
        "\u2029", "\u0009", "\u000A", "\u000B", "\u000C", "\u000D", "\u0085",
    };

    std::vector<nonstd::expected<jsonh_token, std::string>> read_object() noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        // Opening brace
        if (!read_one("{")) {
            // Braceless object
            for (const nonstd::expected<jsonh_token, std::string>& token : read_braceless_object()) {
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
            for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
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
                tokens.push_back(nonstd::unexpected<std::string>("Expected `}` to end object, got end of input"));
                return tokens;
            }

            // Closing brace
            if (next.value() == "}") {
                // End of object
                read();
                tokens.push_back(jsonh_token(json_token_type::end_object));
                return tokens;
            }
            // Property
            else {
                for (const nonstd::expected<jsonh_token, std::string>& token : read_property()) {
                    if (!token) {
                        tokens.push_back(token);
                        return tokens;
                    }
                    tokens.push_back(token);
                }
            }
        }
    }
    std::vector<nonstd::expected<jsonh_token, std::string>> read_braceless_object(std::optional<std::vector<jsonh_token>> property_name_tokens = std::nullopt) noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        // Start of object
        tokens.push_back(jsonh_token(json_token_type::start_object));

        // Initial tokens
        if (property_name_tokens) {
            for (const nonstd::expected<jsonh_token, std::string>& token : read_property(property_name_tokens)) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }

        while (true) {
            // Comments & whitespace
            for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
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
            for (const nonstd::expected<jsonh_token, std::string>& token : read_property()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }
    }
    std::vector<nonstd::expected<jsonh_token, std::string>> read_property(std::optional<std::vector<jsonh_token>> property_name_tokens = std::nullopt) noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        // Property name
        if (property_name_tokens) {
            for (const jsonh_token& token : property_name_tokens.value()) {
                tokens.push_back(token);
            }
        }
        else {
            for (const nonstd::expected<jsonh_token, std::string>& token : read_property_name()) {
                if (!token) {
                    tokens.push_back(token);
                    return tokens;
                }
                tokens.push_back(token);
            }
        }

        // Comments & whitespace
        for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Property value
        for (const nonstd::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Comments & whitespace
        for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
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
    std::vector<nonstd::expected<jsonh_token, std::string>> read_property_name(std::optional<std::string> string = std::nullopt) noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        // String
        if (!string) {
            nonstd::expected<jsonh_token, std::string> string_token = read_string();
            if (!string_token) {
                tokens.push_back(string_token);
                return tokens;
            }
            string = string_token.value().value;
        }

        // Comments & whitespace
        for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Colon
        if (!read_one(":")) {
            tokens.push_back(nonstd::unexpected<std::string>("Expected `:` after property name in object"));
            return tokens;
        }

        // End of property name
        tokens.push_back(jsonh_token(json_token_type::property_name, string.value()));

        return tokens;
    }
    std::vector<nonstd::expected<jsonh_token, std::string>> read_array() noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        // Opening bracket
        if (!read_one("[")) {
            tokens.push_back(nonstd::unexpected<std::string>("Expected `[` to start array"));
            return tokens;
        }
        // Start of array
        tokens.push_back(jsonh_token(json_token_type::start_array));

        while (true) {
            // Comments & whitespace
            for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
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
                tokens.push_back(nonstd::unexpected<std::string>("Expected `]` to end array, got end of input"));
                return tokens;
            }

            // Closing bracket
            if (next.value() == "]") {
                // End of array
                read();
                tokens.push_back(jsonh_token(json_token_type::end_array));
                return tokens;
            }
            // Item
            else {
                for (const nonstd::expected<jsonh_token, std::string>& token : read_item()) {
                    if (!token) {
                        tokens.push_back(token);
                        return tokens;
                    }
                    tokens.push_back(token);
                }
            }
        }
    }
    std::vector<nonstd::expected<jsonh_token, std::string>> read_item() noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        // Element
        for (const nonstd::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Comments & whitespace
        for (const nonstd::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
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
    nonstd::expected<jsonh_token, std::string> read_string() noexcept {
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

        // Read string
        std::string string_builder;

        while (true) {
            std::optional<std::string> next = read();
            if (!next) {
                return nonstd::unexpected<std::string>("Expected end of string, got end of input");
            }

            // Partial end quote was actually part of string
            if (next != start_quote) {
                string_builder.append(end_quote_counter, start_quote_char);
                end_quote_counter = 0;
            }

            // End quote
            if (next.value() == start_quote) {
                end_quote_counter++;
                if (end_quote_counter == start_quote_counter) {
                    break;
                }
            }
            // Escape sequence
            else if (next.value() == "\\") {
                nonstd::expected<std::string, std::string> escape_sequence_result = read_escape_sequence();
                if (!escape_sequence_result) {
                    return nonstd::unexpected<std::string>(escape_sequence_result.error());
                }
                string_builder += escape_sequence_result.value();
            }
            // Literal character
            else {
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
                size_t index = string_builder_reader1.position();
                std::optional<std::string> next = string_builder_reader1.read();
                if (!next) {
                    break;
                }

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
                    size_t index = string_builder_reader2.position();
                    std::optional<std::string> next = string_builder_reader2.read();
                    if (!next) {
                        break;
                    }

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
                    // Pass 3: strip trailing newline -> whitespace
                    string_builder.erase(last_newline_index, string_builder.size() - last_newline_index);

                    // Pass 4: strip leading whitespace -> newline
                    string_builder.erase(0, leading_whitespace_newline_counter);

                    // Condition: skip remaining steps if no trailing whitespace
                    if (trailing_whitespace_counter > 0) {
                        // Pass 5: strip line-leading whitespace
                        utf8_reader string_builder_reader3(string_builder);
                        bool is_line_leading_whitespace = true;
                        int line_leading_whitespace_counter = 0;
                        while (true) {
                            size_t index = string_builder_reader3.position();
                            std::optional<std::string> next = string_builder_reader3.read();
                            if (!next) {
                                break;
                            }

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
    nonstd::expected<jsonh_token, std::string> read_quoteless_string(const std::string& initial_chars = "") noexcept {
        bool is_named_literal_possible = true;

        // Read quoteless string
        std::string string_builder = initial_chars;

        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Escape sequence
            if (next.value() == "\\") {
                read();
                nonstd::expected<std::string, std::string> escape_sequence_result = read_escape_sequence();
                if (!escape_sequence_result) {
                    return nonstd::unexpected<std::string>(escape_sequence_result.error());
                }
                string_builder += escape_sequence_result.value();
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
            return nonstd::unexpected<std::string>("Empty quoteless string");
        }

        // Trim leading whitespace
        utf8_reader string_builder_reader(string_builder);
        while (true) {
            size_t original_position = string_builder_reader.position();
            std::optional<std::string> next = string_builder_reader.read();

            // Non-whitespace
            if (!next || !whitespace_runes.contains(next.value())) {
                string_builder.erase(0, original_position);
                break;
            }
        }
        // Trim trailing whitespace
        utf8_reader string_builder_reader2(string_builder);
        string_builder_reader2.seek(0, std::ios_base::end);
        while (true) {
            size_t original_position = string_builder_reader2.position();
            std::optional<std::string> last = string_builder_reader2.read_reverse();

            // Non-whitespace
            if (!last || !whitespace_runes.contains(last.value())) {
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
            // Peek rune
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
    nonstd::expected<jsonh_token, std::string> read_number_or_quoteless_string() noexcept {
        // Read number
        std::string number_builder;
        nonstd::expected<jsonh_token, std::string> number = read_number(number_builder);
        if (number) {
            // Try read quoteless string starting with number
            std::string whitespace_chars;
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
    nonstd::expected<jsonh_token, std::string> read_number(std::string& number_builder) noexcept {
        // Read sign
        std::optional<std::string> sign = read_any({ "-", "+" });
        if (sign) {
            number_builder += sign.value();
        }

        // Read base
        std::string base_digits = "0123456789";
        bool has_base_specifier = false;
        if (read_one("0")) {
            number_builder += '0';

            std::optional<std::string> hex_base_char = read_any({ "x", "X" });
            if (hex_base_char) {
                number_builder += hex_base_char.value();
                base_digits = "0123456789abcdef";
                has_base_specifier = true;
            }
            else {
                std::optional<std::string> binary_base_char = read_any({ "b", "B" });
                if (binary_base_char) {
                    number_builder += binary_base_char.value();
                    base_digits = "01";
                    has_base_specifier = true;
                }
                else {
                    std::optional<std::string> octal_base_char = read_any({ "o", "O" });
                    if (octal_base_char) {
                        number_builder += octal_base_char.value();
                        base_digits = "01234567";
                        has_base_specifier = true;
                    }
                }
            }
        }

        // Read main number
        nonstd::expected<void, std::string> main_result = read_number_no_exponent(number_builder, base_digits, has_base_specifier);
        if (!main_result) {
            return nonstd::unexpected<std::string>(main_result.error());
        }

        // Hexadecimal exponent
        if (number_builder.back() == 'e' || number_builder.back() == 'E') {
            // Read sign
            std::optional<std::string> exponent_sign = read_any({ "+", "-" });
            if (exponent_sign) {
                number_builder += exponent_sign.value();

                // Read exponent number
                nonstd::expected<void, std::string> exponent_result = read_number_no_exponent(number_builder, base_digits, has_base_specifier);
                if (!exponent_result) {
                    return nonstd::unexpected<std::string>(exponent_result.error());
                }
            }
        }
        // Exponent
        else {
            std::optional<std::string> exponent_char = read_any({ "e", "E" });
            if (exponent_char) {
                number_builder += exponent_char.value();

                // Read sign
                std::optional<std::string> exponent_sign = read_any({ "-", "+" });
                if (exponent_sign) {
                    number_builder += exponent_sign.value();
                }

                // Read exponent number
                nonstd::expected<void, std::string> exponent_result = read_number_no_exponent(number_builder, base_digits, has_base_specifier);
                if (!exponent_result) {
                    return nonstd::unexpected<std::string>(exponent_result.error());
                }
            }
        }

        // End of number
        return jsonh_token(json_token_type::number, number_builder);
    }
    nonstd::expected<void, std::string> read_number_no_exponent(std::string& number_builder, std::string_view base_digits, bool has_base_specifier) noexcept {
        // Leading underscore
        if (!has_base_specifier && peek() == "_") {
            return nonstd::unexpected<std::string>("Leading `_` in number");
        }

        bool is_fraction = false;

        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Digit
            if (base_digits.find(to_ascii_lower(next.value().data())) != std::string::npos) {
                read();
                number_builder += next.value();
            }
            // Dot
            else if (next.value() == ".") {
                read();
                number_builder += next.value();

                // Duplicate dot
                if (is_fraction) {
                    return nonstd::unexpected<std::string>("Duplicate `.` in number");
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
            return nonstd::unexpected<std::string>("Empty number");
        }

        // Ensure at least one digit
        if (number_builder.find_first_not_of(".-+_") == std::string::npos) {
            return nonstd::unexpected<std::string>("Number must have at least one digit");
        }

        // Trailing underscore
        if (number_builder.ends_with('_')) {
            return nonstd::unexpected<std::string>("Trailing `_` in number");
        }

        // End of number
        return nonstd::expected<void, std::string>(); // Success
    }
    nonstd::expected<jsonh_token, std::string> read_primitive_element() noexcept {
        // Peek rune
        std::optional<std::string> next = peek();
        if (!next) {
            return nonstd::unexpected<std::string>("Expected primitive element, got end of input");
        }

        // Number
        if ((next.value() >= "0" && next.value() <= "9") || (next.value() == "-" || next.value() == "+") || next.value() == ".") {
            return read_number_or_quoteless_string();
        }
        // String
        else if (next.value() == "\"" || next.value() == "'") {
            return read_string();
        }
        // Quoteless string (or named literal)
        else {
            return read_quoteless_string();
        }
    }
    std::vector<nonstd::expected<jsonh_token, std::string>> read_comments_and_whitespace() noexcept {
        std::vector<nonstd::expected<jsonh_token, std::string>> tokens = {};

        while (true) {
            // Whitespace
            read_whitespace();

            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Comment
            if (next.value() == "#" || next.value() == "/") {
                tokens.push_back(read_comment());
            }
            // End of comments
            else {
                break;
            }
        }

        return tokens;
    }
    nonstd::expected<jsonh_token, std::string> read_comment() noexcept {
        bool block_comment = false;

        // Hash-style comment
        if (read_one("#")) {
        }
        else if (read_one("/")) {
            // Line-style comment
            if (read_one("/")) {
            }
            // Block-style comment
            else if (read_one("*")) {
                block_comment = true;
            }
            else {
                return nonstd::unexpected<std::string>("Unexpected `/`");
            }
        }
        else {
            return nonstd::unexpected<std::string>("Unexpected character");
        }

        // Read comment
        std::string comment_builder;

        while (true) {
            // Read rune
            std::optional<std::string> next = read();

            if (block_comment) {
                // Error
                if (!next) {
                    return nonstd::unexpected<std::string>("Expected end of block comment, got end of input");
                }
                // End of block comment
                if (next.value() == "*" && read_one("/")) {
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
    nonstd::expected<unsigned int, std::string> read_hex_sequence(size_t length) noexcept {
        std::string hex_chars(length, '\0');

        for (size_t index = 0; index < length; index++) {
            std::optional<std::string> next = read();

            // Hex digit
            if ((next >= "0" && next <= "9") || (next >= "A" && next <= "F") || (next >= "a" && next <= "f")) {
                hex_chars[index] = next.value()[0];
            }
            // Unexpected char
            else {
                return nonstd::unexpected<std::string>("Incorrect number of hexadecimal digits in unicode escape sequence");
            }
        }

        // Parse unicode character from hex digits
        return (unsigned int)std::stoul(hex_chars, nullptr, 16);
    }
    nonstd::expected<std::string, std::string> read_escape_sequence() noexcept {
        std::optional<std::string> escape_char = read();
        if (!escape_char) {
            return nonstd::unexpected<std::string>("Expected escape sequence, got end of input");
        }

        // Reverse solidus
        if (escape_char.value() == "\\") {
            return "\\";
        }
        // Backspace
        else if (escape_char.value() == "b") {
            return "\b";
        }
        // Form feed
        else if (escape_char.value() == "f") {
            return "\f";
        }
        // Newline
        else if (escape_char.value() == "n") {
            return "\n";
        }
        // Carriage return
        else if (escape_char.value() == "r") {
            return "\r";
        }
        // Tab
        else if (escape_char.value() == "t") {
            return "\t";
        }
        // Vertical tab
        else if (escape_char.value() == "v") {
            return "\v";
        }
        // Null
        else if (escape_char.value() == "0") {
            return "\0";
        }
        // Alert
        else if (escape_char.value() == "a") {
            return "\a";
        }
        // Escape
        else if (escape_char.value() == "e") {
            return "\u001b";
        }
        // Unicode hex sequence
        else if (escape_char.value() == "u") {
            return read_hex_escape_sequence(4);
        }
        // Short unicode hex sequence
        else if (escape_char.value() == "x") {
            return read_hex_escape_sequence(2);
        }
        // Long unicode hex sequence
        else if (escape_char.value() == "U") {
            return read_hex_escape_sequence(8);
        }
        // Escaped newline
        else if (newline_runes.contains(escape_char.value())) {
            // Join CR LF
            if (escape_char == "\r") {
                read_one("\n");
            }
            return "";
        }
        // Other
        else {
            return escape_char.value();
        }
    }
    nonstd::expected<std::string, std::string> read_hex_escape_sequence(size_t length) noexcept {
        // Read hex digits & convert to uint
        nonstd::expected<unsigned int, std::string> code_point = read_hex_sequence(length);
        if (!code_point) {
            return nonstd::unexpected<std::string>(code_point.error());
        }

        // High surrogate
        if (is_utf16_high_surrogate(code_point.value())) {
            size_t original_position = position();
            // Escape sequence
            if (read_one("\\")) {
                std::optional<std::string> next = read_any({ "u", "x", "U" });
                // Low surrogate escape sequence
                if (next) {
                    // Read hex sequence
                    nonstd::expected<unsigned int, std::string> low_code_point;
                    if (next == "u") {
                        low_code_point = read_hex_sequence(4);
                    }
                    else if (next == "x") {
                        low_code_point = read_hex_sequence(2);
                    }
                    else if (next == "U") {
                        low_code_point = read_hex_sequence(8);
                    }
                    // Ensure hex sequence read successfully
                    if (!low_code_point) {
                        return nonstd::unexpected<std::string>(low_code_point.error());
                    }
                    // Combine high and low surrogates
                    code_point = utf16_surrogates_to_code_point(code_point.value(), low_code_point.value());
                }
                // Other escape sequence
                else {
                    seek(original_position);
                }
            }
        }

        // Rune
        nonstd::expected<std::string, std::string> rune = code_point_to_utf8(code_point.value());
        if (!rune) {
            return nonstd::unexpected<std::string>(rune.error());
        }
        return rune.value();
    }
    static nonstd::expected<std::string, std::string> code_point_to_utf8(unsigned int code_point) noexcept {
        // Invalid surrogate
        if (code_point >= 0xD800 && code_point <= 0xDFFF) {
            return nonstd::unexpected<std::string>("Invalid code point (surrogate half)");
        }
        // 1-byte UTF-8
        else if (code_point <= 0x7F) {
            return std::string({
                (char)code_point,
            });
        }
        // 2-byte UTF-8
        else if (code_point <= 0x7FF) {
            return std::string({
                (char)(0xC0 | (code_point >> 6)),
                (char)(0x80 | (code_point & 0x3F)),
            });
        }
        // 3-byte UTF-8
        else if (code_point <= 0xFFFF) {
            return std::string({
                (char)(0xE0 | (code_point >> 12)),
                (char)(0x80 | ((code_point >> 6) & 0x3F)),
                (char)(0x80 | (code_point & 0x3F)),
            });
        }
        // 4-byte UTF-8
        else if (code_point <= 0x10FFFF) {
            return std::string({
                (char)(0xF0 | (code_point >> 18)),
                (char)(0x80 | ((code_point >> 12) & 0x3F)),
                (char)(0x80 | ((code_point >> 6) & 0x3F)),
                (char)(0x80 | (code_point & 0x3F)),
            });
        }
        // Invalid UTF-8
        else {
            return nonstd::unexpected<std::string>("Invalid code point (out of range)");
        }
    }
    static unsigned int utf16_surrogates_to_code_point(unsigned int high_surrogate, unsigned int low_surrogate) noexcept {
        return 0x10000 + (((high_surrogate - 0xD800) << 10) | (low_surrogate - 0xDC00));
    }
    static bool is_utf16_high_surrogate(unsigned int code_point) noexcept {
        return code_point >= 0xD800 && code_point <= 0xDBFF;
    }
    static std::string to_ascii_lower(const char* string) noexcept {
        std::string result(string);
        for (char& next : result) {
            if (next <= 'Z' && next >= 'A') {
                next -= ('Z' - 'z');
            }
        }
        return result;
    }
};

}