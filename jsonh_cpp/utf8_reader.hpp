#pragma once

#include <sstream> // for std::istream
#include <vector> // for std::vector
#include <optional> // for std::optional
#include <set> // for std::set

namespace jsonh_cpp {

/// <summary>
/// A reader that reads UTF-8 runes from a UTF-8 input stream.
/// </summary>
class utf8_reader {
public:
    /// <summary>
    /// The byte stream to decode runes from.
    /// </summary>
    std::unique_ptr<std::istream> inner_stream;

    /// <summary>
    /// Constructs a reader that reads UTF-8 runes from a UTF-8 input stream.
    /// </summary>
    utf8_reader(std::unique_ptr<std::istream> stream) noexcept {
        this->inner_stream = std::move(stream);
    }
    /// <summary>
    /// Constructs a reader that reads UTF-8 runes from a UTF-8 input stream.
    /// </summary>
    utf8_reader(std::istream& stream) noexcept
        : utf8_reader(std::unique_ptr<std::istream>(&stream)) {
    }
    /// <summary>
    /// Constructs a reader that reads UTF-8 runes from a UTF-8 string.
    /// </summary>
    utf8_reader(const std::string& string) noexcept
        : utf8_reader(std::make_unique<std::istringstream>(string)) {
    }

    /// <summary>
    /// Returns the current byte position in <see cref="inner_stream"/>.
    /// </summary>
    size_t position() const noexcept {
        return inner_stream->tellg();
    }
    /// <summary>
    /// Sets the current byte position in <see cref="inner_stream"/> relative to the given anchor.
    /// </summary>
    void seek(size_t position, std::ios::seekdir anchor = std::ios::beg) const noexcept {
        inner_stream->seekg(position, anchor);
    }

    /// <summary>
    /// Reads the next UTF-8 rune from <see cref="inner_stream"/>, moving forward by the number of bytes read.
    /// </summary>
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
        int sequence_length = get_utf8_sequence_length((char)first_byte_as_int);

        // Read remaining bytes (up to 3 more)
        std::string bytes((size_t)(1 + sequence_length), '\0');
        bytes[0] = (char)first_byte_as_int;
        inner_stream->read(&bytes[1], sequence_length);

        // Trim excess bytes
        bytes.resize(1 + inner_stream->gcount());
        return bytes;
    }
    /// <summary>
    /// Reads the next UTF-8 rune from <see cref="inner_stream"/>, without moving forward.
    /// </summary>
    std::optional<std::string> peek() const noexcept {
        size_t original_position = position();
        std::optional<std::string> next = read();
        seek(original_position);
        return next;
    }
    /// <summary>
    /// If the next UTF-8 rune is the given option, moves forward by its number of bytes.
    /// </summary>
    bool read_one(std::string option) const noexcept {
        if (peek() == option) {
            read();
            return true;
        }
        return false;
    }
    /// <summary>
    /// If the next UTF-8 rune is one of the given options, moves forward by its number of bytes and returns the option.
    /// </summary>
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

    /// <summary>
    /// Reads the last UTF-8 rune from <see cref="inner_stream"/>, moving backward by the number of bytes read.
    /// </summary>
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

        // Reverse bytes
        std::reverse(bytes.begin(), bytes.end());
        return std::nullopt;
    }
    /// <summary>
    /// Reads the last UTF-8 rune from <see cref="inner_stream"/>, without moving backward.
    /// </summary>
    std::optional<std::string> peek_reverse() const noexcept {
        size_t original_position = position();
        std::optional<std::string> last = read_reverse();
        seek(original_position);
        return last;
    }
    /// <summary>
    /// If the last UTF-8 rune is the given option, moves backward by its number of bytes.
    /// </summary>
    bool read_one_reverse(std::string option) const noexcept {
        if (peek_reverse() == option) {
            read_reverse();
            return true;
        }
        return false;
    }
    /// <summary>
    /// If the last UTF-8 rune is one of the given options, moves backward by its number of bytes and returns the option.
    /// </summary>
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

    /// <summary>
    /// Calculates the byte count of a UTF-8 rune from the bits in its first byte.
    /// </summary>
    /// <returns>
    /// 1 or 2 or 3 or 4.
    /// </returns>
    static int get_utf8_sequence_length(char first_byte) noexcept {
        // https://codegolf.stackexchange.com/a/173577
        return ((first_byte - 160) >> (20 - (first_byte / 16))) + 2;
    }
    /// <summary>
    /// Returns whether the byte is the first (or only) byte of a UTF-8 rune as opposed to a continuation byte.
    /// </summary>
    static bool is_utf8_first_byte(char byte) noexcept {
        return (byte & 0xC0) != 0x80;
    }
};

}