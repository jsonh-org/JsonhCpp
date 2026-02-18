#pragma once

#include <istream>
#include <sstream>
#include <optional>
#include <set>
#include <cstdint>
#include <ios>
#include <memory>
#include <string>
#include <utility>

namespace jsonh_cpp {

/**
* @brief A reader that reads UTF-8 runes from a UTF-8 input stream.
**/
class utf8_reader {
public:
    /**
    * @brief The byte stream to decode runes from.
    **/
    std::unique_ptr<std::istream> inner_stream;
    /**
    * @brief The number of runes read from inner_stream.
    **/
    int64_t char_counter;

    /**
    * @brief Constructs a reader that reads UTF-8 runes from a UTF-8 input stream.
    **/
    explicit utf8_reader(std::unique_ptr<std::istream> stream) noexcept {
        this->inner_stream = std::move(stream);
        this->char_counter = 0;
    }
    /**
    * @brief Constructs a reader that reads UTF-8 runes from a UTF-8 input stream.
    **/
    explicit utf8_reader(std::istream& stream) noexcept
        : utf8_reader(std::unique_ptr<std::istream>(&stream)) {
    }
    /**
    * @brief Constructs a reader that reads UTF-8 runes from a UTF-8 string.
    **/
    explicit utf8_reader(const std::string& string) noexcept
        : utf8_reader(std::make_unique<std::istringstream>(string)) {
    }

    /**
    * @brief Returns the current byte position in @ref inner_stream.
    **/
    size_t position() const noexcept {
        return inner_stream->tellg();
    }
    /**
    * @brief Sets the current byte position in @ref inner_stream relative to the given anchor.
    **/
    void seek(size_t position, std::ios::seekdir anchor = std::ios::beg) const noexcept {
        inner_stream->seekg(position, anchor);
    }

    /**
    * @brief Reads the next UTF-8 rune from @ref inner_stream, moving forward by the number of bytes read.
    **/
    std::optional<std::string> read() const noexcept {
        // Read first byte
        int first_byte_as_int = inner_stream->get();
        if (first_byte_as_int < 0) {
            return std::nullopt;
        }

        // Single byte character performance optimisation
        if (first_byte_as_int <= 127) {
            return std::string({ (char)first_byte_as_int });
        }

        // Get number of bytes in UTF8 character
        uint8_t sequence_length = get_utf8_sequence_length((uint8_t)first_byte_as_int);

        // Read remaining bytes (up to 3 more)
        std::string bytes((size_t)(1 + sequence_length), '\0');
        bytes[0] = (char)first_byte_as_int;
        inner_stream->read(&bytes[1], std::streamsize(sequence_length - 1));

        // Trim excess bytes
        bytes.resize(1 + inner_stream->gcount());
        return bytes;
    }
    /**
    * @brief Reads the next UTF-8 rune from @ref inner_stream, without moving forward.
    **/
    std::optional<std::string> peek() const noexcept {
        size_t original_position = position();
        std::optional<std::string> next = read();
        seek(original_position);
        return next;
    }
    /**
    * @brief If the next UTF-8 rune is the given option, moves forward by its number of bytes.
    **/
    bool read_one(std::string option) const noexcept {
        if (peek() == option) {
            read();
            return true;
        }
        return false;
    }
    /**
    * @brief If the next UTF-8 rune is one of the given options, moves forward by its number of bytes and returns the option.
    **/
    std::optional<std::string> read_any(const std::set<std::string>& options) const noexcept {
        // Peek char
        std::optional<std::string> next = peek();
        if (!next) {
            return std::nullopt;
        }
        // Match option
        if (!options.contains(next.value())) {
            return std::nullopt;
        }
        // Option matched
        read();
        return next;
    }

    /**
    * @brief Reads the last UTF-8 rune from @ref inner_stream, moving backward by the number of bytes read.
    **/
    std::optional<std::string> read_reverse() const noexcept {
        // Read up to 4 bytes
        std::string bytes;
        bytes.reserve(4);
        for (size_t index = 0; index < 4; index++) {
            // Move before last byte
            seek(-1, std::ios::cur);

            // Peek next byte
            int next_as_int = inner_stream->peek();
            if (next_as_int < 0) {
                return std::nullopt;
            }

            // Append byte
            bytes += (char)next_as_int;

            // End if reached first byte
            if (is_utf8_first_byte((char)next_as_int)) {
                return bytes;
            }
        }

        // Never reached first byte
        return std::nullopt;
    }
    /**
    * @brief Reads the last UTF-8 rune from @ref inner_stream, without moving backward.
    **/
    std::optional<std::string> peek_reverse() const noexcept {
        size_t original_position = position();
        std::optional<std::string> last = read_reverse();
        seek(original_position);
        return last;
    }
    /**
    * @brief If the last UTF-8 rune is the given option, moves backward by its number of bytes.
    **/
    bool read_one_reverse(std::string option) const noexcept {
        if (peek_reverse() == option) {
            read_reverse();
            return true;
        }
        return false;
    }
    /**
    * @brief If the last UTF-8 rune is one of the given options, moves backward by its number of bytes and returns the option.
    **/
    std::optional<std::string> read_any_reverse(const std::set<std::string>& options) const noexcept {
        // Peek char
        std::optional<std::string> last = peek_reverse();
        if (!last) {
            return std::nullopt;
        }
        // Match option
        if (!options.contains(last.value())) {
            return std::nullopt;
        }
        // Option matched
        read_reverse();
        return last;
    }

    /**
    * @brief Returns whether the byte is the first (or only) byte of a UTF-8 rune as opposed to a continuation byte.
    **/
    static constexpr bool is_utf8_first_byte(std::uint8_t byte) noexcept {
        return (byte & 0xC0) != 0x80;
    }
    /**
    * @brief Calculates the byte count of a UTF-8 rune from the bits in its first byte.
    *
    * @returns 1 or 2 or 3 or 4.
    **/
    static constexpr uint8_t get_utf8_sequence_length(std::uint8_t first_byte) noexcept {
        // https://codegolf.stackexchange.com/a/173577
        return ((first_byte - 160) >> (20 - (first_byte / 16))) + 2;
    }
};

}