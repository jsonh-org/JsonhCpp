#include <sstream> // for std::istream
#include <vector> // for std::vector
#include <optional> // for std::optional
//#define UTF_CPP_CPLUSPLUS 202302L // for utf8 (C++23)
//#include "utf8cpp/utf8.h" // for utf8

namespace jsonh {

enum struct encoding {
    utf8,
    utf16,
    utf16_bigendian,
    utf32,
    ascii,
};

class stream_rune_reader final {
public:
    /// <summary>
    /// The byte stream to decode runes from.
    /// </summary>
    std::unique_ptr<std::istream> inner_stream;
    /// <summary>
    /// The text encoding used when decoding runes from <see cref="inner_stream"/>.
    /// </summary>
    encoding inner_stream_encoding;

    stream_rune_reader(std::unique_ptr<std::istream> stream, std::optional<encoding> encoding = {}) noexcept {
        this->inner_stream = std::move(stream);
        this->inner_stream_encoding = encoding.value();
    }

    /// <summary>
    /// Frees the resources used by the stream.
    /// </summary>
    ~stream_rune_reader() noexcept {
        inner_stream.reset();
    }

    long position() const noexcept {
        return inner_stream->tellg();
    }
    void set_position(long value) const noexcept {
        inner_stream->seekg(value);
    }

    std::optional<std::string> read() const noexcept {
        long original_position = position();
        // UTF-8
        if (inner_stream_encoding == encoding::utf8) {
            // Read first byte
            int first_byte = inner_stream->get();
            if (first_byte < 0) {
                return std::nullopt;
            }

            // Single byte character performance optimisation
            if (first_byte <= 127) {
                return std::string({ (char)first_byte });
            }

            // Get number of bytes in UTF8 character
            int sequence_length = get_utf8_sequence_length((char)first_byte);

            // Read remaining bytes (up to 3 more)
            std::string bytes;
            bytes.reserve((size_t)(1 + sequence_length));
            bytes[0] = (char)first_byte;
            inner_stream->read(bytes.data() + 1, sequence_length);
            int total_bytes_read = 1 + inner_stream->gcount();

            return bytes;
        }
    }

    /*std::optional<char32_t> peek() const noexcept {
        int next_as_int = stream->peek();
        if (next_as_int < 0) {
            return {};
        }
        char next = (char)next_as_int;
        return next;
    }
    std::optional<char32_t> read() noexcept {
        /*int next_as_int = stream->get();
        if (next_as_int < 0) {
            return {};
        }
        char next = (char)next_as_int;
        char_counter++;
        return next;*/

        /*char next_rune[4] = {};
        for (size_t index = 0; index < 4; index++) {
            std::optional<char> next = read();
            if (!next) {
                return std::nullopt;
            }

            next_rune[index] = next.value();

            if (utf8::is_valid(next_rune)) {
                break;
            }
        }
        utf8::utf8to32(next_rune);
        return next_rune;
    }*/

private:
    /// <summary>
    /// Calculates the 1-byte char count of a single UTF-8 rune from the bits in its first byte.<br/>
    /// The result will be 1, 2, 3 or 4.
    /// </summary>
    static int get_utf8_sequence_length(char first_byte) {
        // https://codegolf.stackexchange.com/a/173577
        return (first_byte - 160 >> 20 - first_byte / 16) + 2;
    }
    /// <summary>
    /// Calculates the 2-byte char count of a single UTF-16 rune from the bits in its first two bytes.<br/>
    /// The result will be 1 or 2.
    /// </summary>
    static int get_utf16_sequence_length(char first_byte, char second_byte, bool is_big_endian = false) {
        unsigned short value = is_big_endian
            ? (unsigned short)((first_byte << 8) | second_byte)  // Big-endian: Most Significant Byte first
            : (unsigned short)((second_byte << 8) | first_byte); // Little-endian: Least Significant Byte first
        bool is_high_surrogate = value >= 0xD800 && value <= 0xDBFF;
        return is_high_surrogate ? 2 : 1;
    }

    /*char32_t decode_next_rune() noexcept {
        int first = stream->get();
        if (first == EOF) return EOF;

        uint8_t byte1 = static_cast<uint8_t>(first);
        char32_t codepoint;

        if (byte1 < 0x80) {
            // 1-byte ASCII
            return byte1;
        }

        int num_bytes = 0;
        if ((byte1 & 0xE0) == 0xC0) {
            codepoint = byte1 & 0x1F;
            num_bytes = 1;
        }
        else if ((byte1 & 0xF0) == 0xE0) {
            codepoint = byte1 & 0x0F;
            num_bytes = 2;
        }
        else if ((byte1 & 0xF8) == 0xF0) {
            codepoint = byte1 & 0x07;
            num_bytes = 3;
        }
        else {
            throw std::runtime_error("Invalid UTF-8 byte");
        }

        for (int i = 0; i < num_bytes; ++i) {
            int next = stream->get();
            if (next == EOF || (next & 0xC0) != 0x80) {
                throw std::runtime_error("Invalid UTF-8 continuation byte");
            }
            codepoint = (codepoint << 6) | (next & 0x3F);
        }

        return codepoint;
    }*/
};

}