#include <sstream> // for std::istream
#include <vector> // for std::vector
#include <optional> // for std::optional
#include <set> // for std::set

namespace jsonh {

class utf8_reader {
public:
    /// <summary>
    /// The byte stream to decode runes from.
    /// </summary>
    std::unique_ptr<std::istream> inner_stream;

    /// <summary>
    /// Constructs a reader that reads UTF-8 runes from a UTF-8 stream.
    /// </summary>
    utf8_reader(std::unique_ptr<std::istream> stream) noexcept {
        this->inner_stream = std::move(stream);
    }
    /// <summary>
    /// Constructs a reader that reads UTF-8 runes from a UTF-8 stream.
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
    /// Constructs a reader that reads UTF-8 runes from a UTF-8 string_view converted to a string.
    /// </summary>
    utf8_reader(const std::string_view& string_view) noexcept
        : utf8_reader(std::string(string_view)) {
    }
    /// <summary>
    /// Constructs a reader that reads UTF-8 runes from a UTF-8 char pointer converted to a string.
    /// </summary>
    utf8_reader(const char* string) noexcept
        : utf8_reader(std::string(string)) {
    }

    size_t position() const noexcept {
        return inner_stream->tellg();
    }
    void seek(size_t position, std::ios::seekdir way = std::ios::beg) const noexcept {
        inner_stream->seekg(position, way);
    }

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
    std::optional<std::string> peek() const noexcept {
        size_t original_position = position();
        std::optional<std::string> next = read();
        seek(original_position);
        return next;
    }
    bool read_one(std::string option) const noexcept {
        if (peek() == option) {
            read();
            return true;
        }
        return false;
    }
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
            if (!is_utf8_continuation((char)next_as_int)) {
                return bytes;
            }
        }

        // Reverse bytes
        std::reverse(bytes.begin(), bytes.end());
        return std::nullopt;
    }
    std::optional<std::string> peek_reverse() const noexcept {
        size_t original_position = position();
        std::optional<std::string> next = read_reverse();
        seek(original_position);
        return next;
    }

private:
    /// <summary>
    /// Calculates the 1-byte char count of a single UTF-8 rune from the bits in its first byte.<br/>
    /// The result will be 1, 2, 3 or 4.
    /// </summary>
    static int get_utf8_sequence_length(char first_byte) {
        // https://codegolf.stackexchange.com/a/173577
        return ((first_byte - 160) >> (20 - (first_byte / 16))) + 2;
    }
    static bool is_utf8_continuation(char byte) {
        return (byte & 192) == 128;
    }
    /*/// <summary>
    /// Calculates the 2-byte char count of a single UTF-16 rune from the bits in its first two bytes.<br/>
    /// The result will be 1 or 2.
    /// </summary>
    static int get_utf16_sequence_length(char first_byte, char second_byte, bool is_big_endian = false) {
        unsigned short value = is_big_endian
            ? (unsigned short)((first_byte << 8) | second_byte)  // Big-endian: Most Significant Byte first
            : (unsigned short)((second_byte << 8) | first_byte); // Little-endian: Least Significant Byte first
        bool is_high_surrogate = value >= 0xD800 && value <= 0xDBFF;
        return is_high_surrogate ? 2 : 1;
    }*/
};

}