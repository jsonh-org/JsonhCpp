#pragma once

#include <string> // for std::string
#include <sstream> // for std::istringstream
#include <set> // for std::set
#include "nlohmann/json.hpp" // for nlohmann::json
#include "result/result.hpp" // for cpp::result
#include "jsonh_error.hpp" // for jsonh::jsonh_error
#include "jsonh_reader_options.hpp" // for jsonh::jsonh_reader_options

using namespace nlohmann;
using namespace cpp;

namespace jsonh {

class jsonh_reader final {
public:
    /// <summary>
    /// The stream to read characters from.
    /// </summary>
    std::istream* stream;
    /// <summary>
    /// The options to use when reading JSONH.
    /// </summary>
    jsonh_reader_options* options;
    /// <summary>
    /// The number of characters read from <see cref="stream"/>.
    /// </summary>
    long char_counter = 0;

    /// <summary>
    /// Constructs a reader that reads JSONH from a stream.
    /// </summary>
    jsonh_reader(std::istream* stream, jsonh_reader_options* options = nullptr) {
        this->stream = stream;
        this->options = (options != nullptr ? options : new jsonh_reader_options());
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a string.
    /// </summary>
    jsonh_reader(std::string* string, jsonh_reader_options* options = nullptr) : jsonh_reader(new std::istringstream(*string), options) {
    }
    /// <summary>
    /// Constructs a reader that reads JSONH from a string.
    /// </summary>
    jsonh_reader(const char* string, jsonh_reader_options* options = nullptr) : jsonh_reader(new std::string(string), options) {
    }
    
    std::vector<result<void, jsonh_error>> read_element() {
        for (result<void, jsonh_error> token : read_comments_and_whitespace()) {

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

        return {};
    }

    /*
    void read_element() {

    }
    */

private:
    const std::set<char32_t> reserved_chars = { U'\\', U',', U':', U'[', U']', U'{', U'}', U'/', U'#', U'"', U'\'' };
    const std::set<char32_t> newline_chars = { U'\n', U'\r', U'\u2028', U'\u2029' };
    const std::set<char32_t> whitespace_chars = { U' ', U'\t', U'\n', '\r' };

    std::vector<result<void, jsonh_error>> read_comments_and_whitespace() {
        std::vector<result<void, jsonh_error>> tokens = {};

        while (true) {
            // Whitespace
            read_whitespace();

            // Comment
            char* next = peek();
            if (*next == '#' || *next == '/') {

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
            char* next = peek();
            if (next == nullptr) {
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
    char* peek() {
        int next_int = stream->peek();
        if (next_int < 0) {
            return nullptr;
        }
        char next = (char)next_int;
        return &next;
    }
    char* read() {
        int next_int = stream->get();
        if (next_int < 0) {
            return nullptr;
        }
        char next = (char)next_int;
        char_counter++;
        return &next;
    }
    bool read_one(char option) {
        if (*peek() == option) {
            read();
            return true;
        }
        return false;
    }
    char* read_any(std::set<char>* options) {
        // Peek char
        char* next = peek();
        if (next == nullptr) {
            return nullptr;
        }
        // Match option
        if (options->contains(*next)) {
            return nullptr;
        }
        // Option matched
        read();
        return next;
    }
};

}