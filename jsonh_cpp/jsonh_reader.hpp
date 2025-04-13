#pragma once

#include <string> // for std::string
#include <sstream> // for std::istringstream
#include <set> // for std::set
#include <stack> // for std::stack
#include <expected> // for std::expected
#include "nlohmann/json.hpp" // for nlohmann::json
#include "jsonh_token.hpp" // for jsonh::jsonh_token
#include "jsonh_reader_options.hpp" // for jsonh::jsonh_reader_options
#include "jsonh_number_parser.hpp" // for jsonh::jsonh_number_parser

using namespace nlohmann;

namespace jsonh {

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
    ~jsonh_reader() {
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

            // End of input
            return std::unexpected("Expected token, got end of input");
        }
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
            std::expected<jsonh_token, std::string>& token = read_primitive_element();

        }

        /*
        while (true) {
            int c = stream->get();
            if (c < 0) {
                break;
            }
            if (c != 'a') {
                return false;
            }
        }
        return true;
        */

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
    std::vector<std::expected<jsonh_token, std::string>> read_property_name(std::optional<std::unique_ptr<std::string>> string = std::nullopt) noexcept {
        std::vector<std::expected<jsonh_token, std::string>> tokens = {};

        // String
        if (!string) {
            std::expected<jsonh_token, std::string> string_token = read_string();
            if (!string_token) {
                tokens.push_back(string_token);
                return tokens;
            }
            string = std::unique_ptr<std::string>(&string_token.value().value);
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
        tokens.push_back(jsonh_token(json_token_type::property_name));

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
        std::string string_builder = "";

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
                std::expected<void, std::string> escape_sequence_result = read_escape_sequence(&string_builder);
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
            for (size_t index = string_builder.length() - 1; index >= 0; index--) {
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
                            int NewlineLength = 1;
                            // Join CR LF
                            if (leading_char == '\r' && string_builder.length() >= 2 && string_builder[1] == '\n') {
                                NewlineLength = 2;
                            }

                            // Remove leading newline
                            string_builder.erase(0, NewlineLength);
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

        while (true) {
            // Read char
            std::optional<char> next = peek();
            if (!next) {
                break;
            }

            // Read escape sequence
            if (next.value() == '\\') {
                read();
                std::expected<void, std::string> escape_sequence_result = read_escape_sequence(&string_builder);
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

            // Ensure not empty
            if (string_builder.empty()) {
                return std::unexpected("Empty quoteless string");
            }

            // Trim trailing whitespace
            int trailing_whitespace_index = -1;
            for (size_t index = string_builder.length() - 1; index >= 0; index--) {
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
        std::string string_builder = "";

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
                    return jsonh_token(json_token_type::comment, string_builder);
                }
            }
            else {
                // End of line comment
                if (!next || newline_chars.contains(next.value())) {
                    return jsonh_token(json_token_type::comment, string_builder);
                }
            }

            // Comment char
            string_builder += next.value();
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
    std::expected<unsigned int, std::string> read_hex_sequence(int length) {
        std::vector<char> hex_chars(length);

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
        unsigned int z;
        return std::from_chars(hex_chars.begin(), hex_chars.end(), &z, 16);
        //return std::stoul(hex_chars, nullptr, 16);
    }
    /*private Result read_escape_character(scoped ref ValueStringBuilder StringBuilder) {
        if (Read() is not char EscapeChar) {
            return new Error("Expected escape sequence, got end of input");
        }

        // Reverse solidus
        if (EscapeChar is '\\') {
            StringBuilder.Append('\\');
        }
        // Backspace
        else if (EscapeChar is 'b') {
            StringBuilder.Append('\b');
        }
        // Form feed
        else if (EscapeChar is 'f') {
            StringBuilder.Append('\f');
        }
        // Newline
        else if (EscapeChar is 'n') {
            StringBuilder.Append('\n');
        }
        // Carriage return
        else if (EscapeChar is 'r') {
            StringBuilder.Append('\r');
        }
        // Tab
        else if (EscapeChar is 't') {
            StringBuilder.Append('\t');
        }
        // Vertical tab
        else if (EscapeChar is 'v') {
            StringBuilder.Append('\v');
        }
        // Null
        else if (EscapeChar is '0') {
            StringBuilder.Append('\0');
        }
        // Alert
        else if (EscapeChar is 'a') {
            StringBuilder.Append('\a');
        }
        // Escape
        else if (EscapeChar is 'e') {
            StringBuilder.Append('\e');
        }
        // Unicode hex sequence
        else if (EscapeChar is 'u') {
            if (!ReadHexSequence(4).TryGetValue(out uint Result, out Error Error)) {
                return Error;
            }
            StringBuilder.Append((char)Result);
        }
        // Short unicode hex sequence
        else if (EscapeChar is 'x') {
            if (!ReadHexSequence(2).TryGetValue(out uint Result, out Error Error)) {
                return Error;
            }
            StringBuilder.Append((char)Result);
        }
        // Long unicode hex sequence
        else if (EscapeChar is 'U') {
            if (!ReadHexSequence(8).TryGetValue(out uint Result, out Error Error)) {
                return Error;
            }
            StringBuilder.Append((Rune)Result);
        }
        // Escaped newline
        else if (NewlineChars.Contains(EscapeChar)) {
            // Join CR LF
            if (EscapeChar is '\r') {
                ReadOne('\n');
            }
        }
        // Other
        else {
            StringBuilder.Append(EscapeChar);
        }
        return Result.Success;
    }*/
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
        if (options.contains(next.value())) {
            return {};
        }
        // Option matched
        read();
        return next;
    }
};

}