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
    jsonh_reader(std::unique_ptr<std::istream> stream, jsonh_reader_options options = jsonh_reader_options()) {
        this->stream = std::move(stream);
        this->options = options;
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a string.
    /// </summary>
    jsonh_reader(const std::string& string, jsonh_reader_options options = jsonh_reader_options())
        : jsonh_reader(std::make_unique<std::istringstream>(string), options) {
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a string.
    /// </summary>
    jsonh_reader(const char* string, jsonh_reader_options options = jsonh_reader_options())
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
    std::expected<json, std::string_view> parse_element() {
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

        for (std::expected<jsonh_token, std::string_view>& token_result : read_element()) {
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
    
    std::vector<std::expected<jsonh_token, std::string_view>> read_element() {
        std::vector<std::expected<jsonh_token, std::string_view>> tokens = {};

        // Comments & whitespace
        for (std::expected<jsonh_token, std::string_view>& token : read_comments_and_whitespace()) {
            if (!token) {
                tokens.push_back(token);
                return tokens;
            }
            tokens.push_back(token);
        }

        // Peek char
        std::optional<char> next = peek();

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

    /*
    void read_element() {

    }
    */

private:
    const std::set<char32_t> reserved_chars = { U'\\', U',', U':', U'[', U']', U'{', U'}', U'/', U'#', U'"', U'\'' };
    const std::set<char32_t> newline_chars = { U'\n', U'\r', U'\u2028', U'\u2029' };
    const std::set<char32_t> whitespace_chars = { U' ', U'\t', U'\n', '\r' };

    std::vector<std::expected<jsonh_token, std::string_view>> read_comments_and_whitespace() {
        std::vector<std::expected<jsonh_token, std::string_view>> tokens = {};

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
    std::expected<jsonh_token, std::string_view> read_comment() {
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
    void read_whitespace() {
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
    std::optional<char> peek() const {
        int next_int = stream->peek();
        if (next_int < 0) {
            return {};
        }
        char next = (char)next_int;
        return next;
    }
    std::optional<char> read() {
        int next_int = stream->get();
        if (next_int < 0) {
            return {};
        }
        char next = (char)next_int;
        char_counter++;
        return next;
    }
    bool read_one(char option) {
        if (peek() == option) {
            read();
            return true;
        }
        return false;
    }
    std::optional<char> read_any(const std::set<char>& options) {
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