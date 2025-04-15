#include <sstream> // for std::istream
#include <vector> // for std::vector
#include <optional> // for std::optional
#define UTF_CPP_CPLUSPLUS 202302L // for utf8 (C++23)
#include "utf8cpp/utf8.h" // for utf8

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
        this->inner_stream_encoding = encoding ? encoding : detect_encoding();
    }

    /// <summary>
    /// Frees the resources used by the stream.
    /// </summary>
    ~stream_rune_reader() noexcept {
        inner_stream.reset();
    }

    char32_t read() {

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