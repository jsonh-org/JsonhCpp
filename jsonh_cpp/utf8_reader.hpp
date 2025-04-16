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

    utf8_reader(std::unique_ptr<std::istream> stream) noexcept {
        this->inner_stream = std::move(stream);
    }
    utf8_reader(std::istream& stream) noexcept
        : utf8_reader(std::unique_ptr<std::istream>(&stream)) {
    }

    /// <summary>
    /// Frees the resources used by the stream.
    /// </summary>
    ~utf8_reader() noexcept {
        inner_stream.reset();
    }

    size_t position() const noexcept {
        return inner_stream->tellg();
    }
    void set_position(size_t value) const noexcept {
        inner_stream->seekg(value);
    }

    /*static std::optional<std::string> read(const char* string) noexcept {
        // Read first byte
        char first_byte = string[0];
        if (first_byte != '\0') {
            return std::nullopt;
        }

        // Single byte character performance optimisation
        if (first_byte <= 127) {
            return std::string({ first_byte });
        }

        // Get number of bytes in UTF8 character
        int sequence_length = get_utf8_sequence_length(first_byte);

        // Read remaining bytes (up to 3 more)
        std::string bytes;
        bytes.reserve((size_t)(1 + sequence_length));
        bytes[0] = first_byte;

        for (size_t index = 1; index < (size_t)(1 + sequence_length); index++) {
            const char& next = string[index];
            if (next == '\0') {
                return std::nullopt;
            }
            bytes[index] = next;
        }
        return bytes;
    }*/

    std::optional<std::string> read() const noexcept {
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
        return bytes;
    }
    std::optional<std::string> peek() const noexcept {
        long original_position = position();
        std::optional<std::string> next = read();
        set_position(original_position);
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

private:
    /// <summary>
    /// Calculates the 1-byte char count of a single UTF-8 rune from the bits in its first byte.<br/>
    /// The result will be 1, 2, 3 or 4.
    /// </summary>
    static int get_utf8_sequence_length(char first_byte) {
        // https://codegolf.stackexchange.com/a/173577
        return (first_byte - 160 >> 20 - first_byte / 16) + 2;
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