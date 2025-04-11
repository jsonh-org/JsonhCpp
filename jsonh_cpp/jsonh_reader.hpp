#pragma once

#include <string> // for std::string
#include <sstream> // for std::istringstream
#include <set> // for std::set
#include <expected> // for std::expected
#include "nlohmann/json.hpp" // for nlohmann::json
#include "jsonh_error.hpp" // for jsonh::jsonh_error
#include "jsonh_token.hpp" // for jsonh::jsonh_token
#include "jsonh_reader_options.hpp" // for jsonh::jsonh_reader_options

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
    
    std::vector<std::expected<jsonh_token, jsonh_error>> read_element() {
        std::vector<std::expected<jsonh_token, jsonh_error>> tokens = {};

        // Comments & whitespace
        for (std::expected<jsonh_token, jsonh_error>& token : read_comments_and_whitespace()) {
            if (!token.has_value()) {
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

    std::vector<std::expected<jsonh_token, jsonh_error>> read_comments_and_whitespace() {
        std::vector<std::expected<jsonh_token, jsonh_error>> tokens = {};

        while (true) {
            // Whitespace
            read_whitespace();

            // Comment
            std::optional<char> next = peek();
            if (next == '#' || next == '/') {

                //yield return ReadComment();
            }
            // End of comments
            else {
                break;
            }
        }

        return tokens;
    }
    /*
    Result<JsonhToken> ReadComment() {
        bool BlockComment = false;

        // Hash-style comment
        if (ReadOne('#')) {
        }
        else if (ReadOne('/')) {
            // Line-style comment
            if (ReadOne('/')) {
            }
            // Block-style comment
            else if (ReadOne('*')) {
                BlockComment = true;
            }
            else {
                return new Error("Unexpected '/'");
            }
        }
        else {
            return new Error("Unexpected character");
        }

        // Read comment
        using ValueStringBuilder StringBuilder = new(stackalloc char[64]);

        while (true) {
            // Read char
            char ? Char = Read();

            if (BlockComment) {
                // Error
                if (Char is null) {
                    return new Error("Expected end of block comment, got end of input");
                }
                // End of block comment
                if (Char is '*' && ReadOne('/')) {
                    return new JsonhToken(this, JsonTokenType.Comment, StringBuilder.ToString());
                }
            }
            else {
                // End of line comment
                if (Char is null || NewlineChars.Contains(Char.Value)) {
                    return new JsonhToken(this, JsonTokenType.Comment, StringBuilder.ToString());
                }
            }

            // Comment char
            StringBuilder.Append(Char.Value);
        }
    }
    */
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