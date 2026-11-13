// JsonhCpp (JSON for Humans)
// Version: 12.2
// Link: https://github.com/jsonh-org/JsonhCpp
// License: MIT

#ifndef JSONH_CPP_JSONH_CPP_H
#define JSONH_CPP_JSONH_CPP_H


/*** Start of inlined file: jsonh_reader.hpp ***/
#ifndef JSONH_CPP_JSONH_READER_H
#define JSONH_CPP_JSONH_READER_H

#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <stack>
#include <optional>
#include <algorithm>
#include <ios>
#include <istream>
#include <memory>
#include <string_view>
#include <utility>
#include <cstdint>
#include <generator>
#include <expected>

/*** Start of inlined file: json_element.hpp ***/
#ifndef JSONH_CPP_JSON_ELEMENT_H
#define JSONH_CPP_JSON_ELEMENT_H

#include <cstdint>
#include <vector>
#include <map>
#include <variant>
#include <string>
#include <utility>
#include <type_traits>

namespace jsonh_cpp {

/**
* @brief The type of a @ref json_element.
**/
enum struct json_element_type : uint8_t {
    /**
    * @brief A null value.
    *
    * Example: @c null
    **/
    null = 0,
    /**
    * @brief A boolean.
    *
    * Example: @c true
    **/
    boolean = 1,
    /**
    * @brief A number.
    *
    * Example: @c 10
    **/
    number = 2,
    /**
    * @brief A string.
    *
    * Example: @c "value"
    **/
    string = 3,
    /**
    * @brief An array.
    *
    * Example: @c [1,2,3]
    **/
    array = 4,
    /**
    * @brief An object.
    *
    * Example: @c {"a":"b"}
    **/
    object = 5,
};

/**
* @brief A single JSON element.
**/
class json_element {
public:
    /**
    * @brief A null value.
    *
    * Example: @c null
    **/
    using json_null_t = std::nullptr_t;
    /**
    * @brief A boolean.
    *
    * Example: @c true
    **/
    using json_boolean_t = bool;
    /**
    * @brief A number.
    *
    * Example: @c 10
    **/
    using json_number_t = double;
    /**
    * @brief A string.
    *
    * Example: @c "value"
    **/
    using json_string_t = std::string;
    /**
    * @brief An array.
    *
    * Example: @c [1,2,3]
    **/
    using json_array_t = std::vector<json_element>;
    /**
    * @brief An object.
    *
    * Example: @c {"a":"b"}
    **/
    using json_object_t = std::map<json_string_t, json_element>;
    /**
    * @brief Any JSON element.
    *
    * This variant's types match @ref json_element_type.
    **/
    using json_element_t = std::variant<
        json_null_t, // 0
        json_boolean_t, // 1
        json_number_t, // 2
        json_string_t, // 3
        json_array_t, // 4
        json_object_t // 5
    >;

    /**
    * @brief Constructs a JSON element with a value of null.
    **/
    explicit json_element() noexcept {
        this->value = nullptr;
    }
    /**
    * @brief Constructs a JSON element with the given value.
    **/
    explicit json_element(json_element_t value) noexcept {
        this->value = std::move(value);
    }
    /**
    * @brief Constructs a JSON element with the given value.
    *
    * Supports @ref json_element, @ref json_element_t, @ref json_null_t, @ref json_boolean_t, @ref json_number_t, @ref json_string_t,
    * @ref json_array_t, @ref json_object_t, types convertible to "const char*", arithmetic types, types like @ref std::map, and
    * types like @ref std::vector.
    **/
    template<typename T>
    json_element(const T& value) noexcept {
        // Get the underlying type of T
        using U = std::decay_t<T>;

        // Cast from json_element
        if constexpr (std::is_same_v<U, json_element>) {
            *this = value;
        }
        // Cast from json_element_t
        else if constexpr (std::is_same_v<U, json_element_t>) {
            this->value = value;
        }
        // Cast from json_null_t
        else if constexpr (std::is_same_v<U, json_null_t>) {
            this->value = json_element_t(value);
        }
        // Cast from json_boolean_t
        else if constexpr (std::is_same_v<U, json_boolean_t>) {
            this->value = json_element_t(value);
        }
        // Cast from json_number_t
        else if constexpr (std::is_same_v<U, json_number_t>) {
            this->value = json_element_t(value);
        }
        // Cast from json_string_t
        else if constexpr (std::is_same_v<U, json_string_t>) {
            this->value = json_element_t(value);
        }
        // Cast from json_array_t
        else if constexpr (std::is_same_v<U, json_array_t>) {
            this->value = json_element_t(value);
        }
        // Cast from json_object_t
        else if constexpr (std::is_same_v<U, json_object_t>) {
            this->value = json_element_t(value);
        }
        // Cast from const char*
        else if constexpr (std::is_convertible_v<U, const char*>) {
            this->value = json_element_t(json_string_t(value));
        }
        // Cast from numeric type
        else if constexpr (std::is_arithmetic_v<U>) {
            this->value = json_element_t(json_number_t(value));
        }
        // Cast from map-like type
        else if constexpr (requires(U result) { typename U::key_type; typename U::mapped_type; result.begin(); result.end(); }) {
            this->value = json_object_t();
            for (const auto& [map_key, map_value] : value) {
                as_object().emplace(static_cast<json_string_t>(map_key), json_element(map_value));
            }
        }
        // Cast from vector-like type
        else if constexpr (requires(U result) { typename U::value_type; result.begin(); result.end(); }) {
            this->value = json_array_t();
            for (const auto& array_item : value) {
                as_array().emplace_back(json_element(array_item));
            }
        }
        // Unsupported cast
        else {
            static_assert(std::is_same_v<U, void>, "Unsupported conversion from T to json_element");
        }
    }

    /**
    * @brief Returns the type of the JSON element as a @ref json_element_type.
    **/
    json_element_type type() const noexcept {
        return static_cast<json_element_type>(value.index());
    }

    /**
    * @brief Returns true if the JSON element is null (@ref json_null_t).
    **/
    bool is_null() const noexcept {
        return std::holds_alternative<json_null_t>(value);
    }
    /**
    * @brief Returns true if the JSON element is a boolean (@ref json_boolean_t).
    **/
    bool is_boolean() const noexcept {
        return std::holds_alternative<json_boolean_t>(value);
    }
    /**
    * @brief Returns true if the JSON element is a number (@ref json_number_t).
    **/
    bool is_number() const noexcept {
        return std::holds_alternative<json_number_t>(value);
    }
    /**
    * @brief Returns true if the JSON element is a string (@ref json_string_t).
    **/
    bool is_string() const noexcept {
        return std::holds_alternative<json_string_t>(value);
    }
    /**
    * @brief Returns true if the JSON element is an array (@ref json_array_t).
    **/
    bool is_array() const noexcept {
        return std::holds_alternative<json_array_t>(value);
    }
    /**
    * @brief Returns true if the JSON element is an object (@ref json_object_t).
    **/
    bool is_object() const noexcept {
        return std::holds_alternative<json_object_t>(value);
    }

    /**
    * @brief Casts the JSON element to null (@ref json_null_t).
    **/
    json_null_t as_null() const noexcept {
        return std::get<json_null_t>(value);
    }
    /**
    * @brief Casts the JSON element to a boolean (@ref json_boolean_t).
    **/
    json_boolean_t as_boolean() const noexcept {
        return std::get<json_boolean_t>(value);
    }
    /**
    * @brief Casts the JSON element to a number (@ref json_number_t).
    **/
    json_number_t as_number() const noexcept {
        return std::get<json_number_t>(value);
    }
    /**
    * @brief Casts the JSON element to a string (@ref json_string_t).
    **/
    json_string_t& as_string() noexcept {
        return std::get<json_string_t>(value);
    }
    /**
    * @brief Casts the JSON element to a string (@ref json_string_t).
    **/
    const json_string_t& as_string() const noexcept {
        return std::get<json_string_t>(value);
    }
    /**
    * @brief Casts the JSON element to an array (@ref json_array_t).
    **/
    json_array_t& as_array() noexcept {
        return std::get<json_array_t>(value);
    }
    /**
    * @brief Casts the JSON element to an array (@ref json_array_t).
    **/
    const json_array_t& as_array() const noexcept {
        return std::get<json_array_t>(value);
    }
    /**
    * @brief Casts the JSON element to an object (@ref json_object_t).
    **/
    json_object_t& as_object() noexcept {
        return std::get<json_object_t>(value);
    }
    /**
    * @brief Casts the JSON element to an object (@ref json_object_t).
    **/
    const json_object_t& as_object() const noexcept {
        return std::get<json_object_t>(value);
    }
    /**
    * @brief Casts the JSON element to the given type.
    *
    * Supports @ref json_element, @ref json_element_t, @ref json_null_t, @ref json_boolean_t, @ref json_number_t, @ref json_string_t,
    * @ref json_array_t, @ref json_object_t, arithmetic types, types like @ref std::map, and types like @ref std::vector.
    **/
    template<typename T>
    T as() const noexcept {
        // Cast to json_element
        if constexpr (std::is_same_v<T, json_element>) {
            return *this;
        }
        // Cast to json_element_t
        else if constexpr (std::is_same_v<T, json_element_t>) {
            return value;
        }
        // Cast to json_null_t
        else if constexpr (std::is_same_v<T, json_null_t>) {
            return as_null();
        }
        // Cast to json_boolean_t
        else if constexpr (std::is_same_v<T, json_boolean_t>) {
            return as_boolean();
        }
        // Cast to json_number_t
        else if constexpr (std::is_same_v<T, json_number_t>) {
            return as_number();
        }
        // Cast to json_string_t
        else if constexpr (std::is_same_v<T, json_string_t>) {
            return as_string();
        }
        // Cast to json_array_t
        else if constexpr (std::is_same_v<T, json_array_t>) {
            return as_array();
        }
        // Cast to json_object_t
        else if constexpr (std::is_same_v<T, json_object_t>) {
            return as_object();
        }
        // Cast to numeric type
        else if constexpr (std::is_arithmetic_v<T>) {
            return T(as_number());
        }
        // Cast to map-like type
        else if constexpr (requires(T result) { typename T::key_type; typename T::mapped_type; result.begin(); result.end(); }) {
            T result = {};
            for (const auto& [map_key, map_value] : as_object()) {
                result.emplace(map_key, map_value.as<typename T::mapped_type>());
            }
            return result;
        }
        // Cast to vector-like type
        else if constexpr (requires(T result) { typename T::value_type; result.begin(); result.end(); }) {
            T result = {};
            for (const auto& array_item : as_array()) {
                result.emplace_back(array_item.as<typename T::value_type>());
            }
            return result;
        }
        // Unsupported cast
        else {
            static_assert(std::is_same_v<T, void>, "Unsupported conversion from json_element to T");
        }
    }

private:
    /**
    * @brief The value stored inside the @ref json_element.
    **/
    json_element_t value;
};

};

#endif // JSONH_CPP_JSON_ELEMENT_H
/*** End of inlined file: json_element.hpp ***/



/*** Start of inlined file: jsonh_token.hpp ***/
#ifndef JSONH_CPP_JSONH_TOKEN_H
#define JSONH_CPP_JSONH_TOKEN_H

#include <string>

/*** Start of inlined file: json_token_type.hpp ***/
#ifndef JSONH_CPP_JSON_TOKEN_TYPE_H
#define JSONH_CPP_JSON_TOKEN_TYPE_H

#include <cstdint>

namespace jsonh_cpp {

/**
* @brief The types of tokens that make up a JSON document.
**/
enum struct json_token_type : uint8_t {
    /**
    * @brief Indicates that there is no value (not to be confused with null).
    **/
    none = 0,
    /**
    * @brief The start of an object.
    *
    * Example: @c {
    **/
    start_object = 1,
    /**
    * @brief The end of an object.
    *
    * Example: @c }
    **/
    end_object = 2,
    /**
    * @brief The start of an array.
    *
    * Example: @c [
    **/
    start_array = 3,
    /**
    * @brief The end of an array.
    *
    * Example: @c ]
    **/
    end_array = 4,
    /**
    * @brief A property name in an object.
    *
    * Example: @c "key":
    **/
    property_name = 5,
    /**
    * @brief A comment.
    *
    * Example: @c //comment
    **/
    comment = 6,
    /**
    * @brief A string.
    *
    * Example: @c "value"
    **/
    string = 7,
    /**
    * @brief A number.
    *
    * Example: @c 10
    **/
    number = 8,
    /**
    * @brief A true boolean.
    *
    * Example: @c true
    **/
    true_bool = 9,
    /**
    * @brief A false boolean.
    *
    * Example: @c false
    **/
    false_bool = 10,
    /**
    * @brief A null value.
    *
    * Example: @c null
    **/
    null = 11,
};

}

#endif // JSONH_CPP_JSON_TOKEN_TYPE_H
/*** End of inlined file: json_token_type.hpp ***/


namespace jsonh_cpp {

/**
* @brief A single JSONH token with a @ref json_token_type.
**/
struct jsonh_token {
    /**
    * @brief The type of the token.
    **/
    json_token_type json_type;
    /**
    * @brief The value of the token, or an empty string.
    **/
    std::string value;

    /**
    * @brief Constructs a single JSONH token.
    **/
    jsonh_token(json_token_type json_type, std::string value = "") noexcept {
        this->json_type = json_type;
        this->value = value;
    }
};

}

#endif // JSONH_CPP_JSONH_TOKEN_H
/*** End of inlined file: jsonh_token.hpp ***/


/*** Start of inlined file: jsonh_reader_options.hpp ***/
#ifndef JSONH_CPP_JSONH_READER_OPTIONS_H
#define JSONH_CPP_JSONH_READER_OPTIONS_H


/*** Start of inlined file: jsonh_version.hpp ***/
#ifndef JSONH_CPP_JSONH_VERSION_H
#define JSONH_CPP_JSONH_VERSION_H

namespace jsonh_cpp {

/**
* @brief The major versions of the JSONH specification.
**/
enum struct jsonh_version {
    /**
    * @brief Indicates that the latest version should be used (currently @ref v2).
    **/
    latest = 0,
    /**
    * @brief Version 1 of the specification, released 2025/03/19.
    **/
    v1 = 1,
    /**
    * @brief Version 2 of the specification, released 2025/11/19.
    **/
    v2 = 2,
};

}

#endif // JSONH_CPP_JSONH_VERSION_H
/*** End of inlined file: jsonh_version.hpp ***/

namespace jsonh_cpp {

/**
* @brief Options for a jsonh_reader.
**/
struct jsonh_reader_options {
    /**
    * @brief Specifies the major version of the JSONH specification to use.
    **/
    jsonh_version version = jsonh_version::latest;
    /**
    * @brief Enables/disables checks for exactly one element when parsing.
    *
    * @code{.jsonh}
    * "cat"
    * "dog" // Error: Expected single element
    * @endcode
    *
    * This option does not apply when reading elements, only when parsing elements.
    **/
    bool parse_single_element = false;
    /**
    * @brief Sets the maximum recursion depth allowed when reading JSONH.
    *
    * @code{.jsonh}
    * // Max depth: 2
    * {
    *   a: {
    *     b: {
    *       // Error: Exceeded max depth
    *     }
    *   }
    * }
    * @endcode
    *
    * The default value is 64 to defend against DOS attacks.
    **/
    int max_depth = 64;
    /**
    * @brief Enables/disables parsing unclosed inputs.
    *
    * @code{.jsonh}
    * {
    *   "key": "val
    * @endcode
    *
    * This is potentially useful for large language models that stream responses.
    * Only some tokens can be incomplete in this mode, so it should not be relied upon.
    **/
    bool incomplete_inputs = false;

    /**
    * @brief Returns whether @ref version is greater than or equal to @ref minimum_version.
    **/
    bool supports_version(jsonh_version minimum_version) const noexcept {
        const jsonh_version latest_version = jsonh_version::v2;

        jsonh_version options_version = version == jsonh_version::latest ? latest_version : version;
        jsonh_version given_version = minimum_version == jsonh_version::latest ? latest_version : minimum_version;

        return options_version >= given_version;
    }
};

}

#endif // JSONH_CPP_JSONH_READER_OPTIONS_H
/*** End of inlined file: jsonh_reader_options.hpp ***/


/*** Start of inlined file: jsonh_number_parser.hpp ***/
#ifndef JSONH_CPP_JSONH_NUMBER_PARSER_H
#define JSONH_CPP_JSONH_NUMBER_PARSER_H

#include <string>
#include <string_view>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <expected>

namespace jsonh_cpp {

/**
* @brief Methods for parsing JSONH numbers (double).
*
* Unlike jsonh_reader.read_element, minimal validation is done here. Ensure the input is valid.
**/
class jsonh_number_parser final {
public:
    /**
    * @brief Converts a JSONH number to a base-10 real.
    * For example:
    *
    * Input: @c +5.2e3.0
    *
    * Output: @c 5200
    **/
    static std::expected<double, std::string> parse(std::string jsonh_number) noexcept {
        // Remove underscores
        std::erase(jsonh_number, '_');
        std::string_view digits = jsonh_number;

        // Get sign
        int sign = 1;
        if (digits.starts_with('-')) {
            sign = -1;
            digits = digits.substr(1);
        }
        else if (digits.starts_with('+')) {
            sign = 1;
            digits = digits.substr(1);
        }

        // Decimal
        std::string base_digits = "0123456789";
        // Hexadecimal
        if (digits.starts_with("0x") || digits.starts_with("0X")) {
            base_digits = "0123456789abcdef";
            digits = digits.substr(2);
        }
        // Binary
        else if (digits.starts_with("0b") || digits.starts_with("0B")) {
            base_digits = "01";
            digits = digits.substr(2);
        }
        // Octal
        else if (digits.starts_with("0o") || digits.starts_with("0O")) {
            base_digits = "01234567";
            digits = digits.substr(2);
        }

        // Parse number with base digits
        std::expected<double, std::string> number = parse_fractional_number_with_exponent(digits, base_digits);
        if (!number) {
            return std::unexpected(number.error());
        }

        // Apply sign
        if (sign != 1) {
            number.value() *= sign;
        }
        return number;
    }

private:
    /**
    * @brief Converts a fractional number with an exponent (e.g. @c 12.3e4.5) from the given base (e.g. @c 01234567) to a base-10 real.
    **/
    static std::expected<double, std::string> parse_fractional_number_with_exponent(std::string_view digits, std::string_view base_digits) noexcept {
        // Find exponent
        size_t exponent_index = std::string::npos;
        // Hexadecimal exponent
        if (base_digits.find('e') != std::string::npos) {
            for (size_t index = 0; index < digits.size(); index++) {
                if (digits[index] != 'e' && digits[index] != 'E') {
                    continue;
                }
                if (index + 1 >= digits.size() || (digits[index + 1] != '+' && digits[index + 1] != '-')) {
                    continue;
                }
                exponent_index = index;
                break;
            }
        }
        // Exponent
        else {
            exponent_index = digits.find_first_of("eE");
        }

        // If no exponent then parse real
        if (exponent_index == std::string::npos) {
            return parse_fractional_number(digits, base_digits);
        }

        // Get mantissa and exponent
        std::string_view mantissa_part = digits.substr(0, exponent_index);
        std::string_view exponent_part = digits.substr(exponent_index + 1);

        // Parse mantissa and exponent
        std::expected<double, std::string> mantissa = parse_fractional_number(mantissa_part, base_digits);
        if (!mantissa) {
            return std::unexpected(mantissa.error());
        }
        std::expected<double, std::string> exponent = parse_fractional_number(exponent_part, base_digits);
        if (!exponent) {
            return std::unexpected(exponent.error());
        }

        // Multiply mantissa by 10 ^ exponent
        return mantissa.value() * pow(10, exponent.value());
    }
    /**
    * @brief Converts a fractional number (e.g. @c 123.45) from the given base (e.g. @c 01234567) to a base-10 real.
    **/
    static std::expected<double, std::string> parse_fractional_number(std::string_view digits, std::string_view base_digits) noexcept {
        // Optimization for base-10 digits
        if (base_digits == "0123456789") {
            return string_to_number(std::string(digits));
        }

        // Find dot
        size_t dot_index = digits.find('.');
        // If no dot then parse integer
        if (dot_index == std::string::npos) {
            return parse_whole_number(digits, base_digits);
        }

        // Get parts of number
        std::string_view whole_part = digits.substr(0, dot_index);
        std::string_view fraction_part = digits.substr(dot_index + 1);

        // Parse parts of number
        std::expected<double, std::string> whole = parse_whole_number(whole_part, base_digits);
        if (!whole) {
            return std::unexpected(whole.error());
        }

        // Add each column of fraction digits
        double fraction = 0;
        for (size_t index = fraction_part.size() - 1; index != std::string::npos; --index) {
            // Get current digit
            char digit_char = fraction_part[index];
            size_t digit_int = base_digits.find((char)std::tolower(digit_char));

            // Ensure digit is valid
            if (digit_int == std::string::npos) {
                return std::unexpected("Invalid digit");
            }

            // Add value of column
            fraction = (fraction + digit_int) / base_digits.size();
        }

        // Combine whole and fraction
        return whole.value() + fraction;
    }
    /**
    * @brief Converts a whole number (e.g. @c 12345) from the given base (e.g. @c 01234567) to a base-10 integer.
    **/
    static std::expected<double, std::string> parse_whole_number(std::string_view digits, std::string_view base_digits) noexcept {
        // Optimization for base-10 digits
        if (base_digits == "0123456789") {
            return string_to_number(std::string(digits));
        }

        // Get sign
        int sign = 1;
        if (digits.starts_with('-')) {
            sign = -1;
            digits = digits.substr(1);
        }
        else if (digits.starts_with('+')) {
            sign = 1;
            digits = digits.substr(1);
        }

        // Add each column of digits
        double integer = 0;
        for (size_t index = 0; index < digits.size(); index++) {
            // Get current digit
            char digit_char = digits[index];
            size_t digit_int = base_digits.find((char)std::tolower(digit_char));

            // Ensure digit is valid
            if (digit_int == std::string::npos) {
                return std::unexpected("Invalid digit");
            }

            // Add value of column
            integer = (integer * base_digits.size()) + digit_int;
        }

        // Apply sign
        if (sign != 1) {
            integer *= sign;
        }
        return integer;
    }
    static double string_to_number(std::string value) {
        return std::strtod(value.c_str(), nullptr);
    }
};

}

#endif // JSONH_CPP_JSONH_NUMBER_PARSER_H
/*** End of inlined file: jsonh_number_parser.hpp ***/


/*** Start of inlined file: utf8_reader.hpp ***/
#ifndef JSONH_CPP_UTF8_READER_H
#define JSONH_CPP_UTF8_READER_H

#include <istream>
#include <sstream>
#include <optional>
#include <set>
#include <cstdint>
#include <ios>
#include <memory>
#include <string>
#include <utility>
#include <algorithm>

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
    * @brief Constructs a reader that reads UTF-8 runes from a UTF-8 string.
    **/
    explicit utf8_reader(const std::string& string) noexcept
        : utf8_reader(std::make_unique<std::istringstream>(string)) {
    }

    /**
    * @brief Returns the current byte position in @ref inner_stream.
    **/
    std::streampos position() const noexcept {
        return inner_stream->tellg();
    }
    /**
    * @brief Sets the current byte position in @ref inner_stream.
    **/
    void seek(std::streampos position) const noexcept {
        inner_stream->seekg(position);
    }
    /**
    * @brief Sets the current byte position in @ref inner_stream relative to the given anchor (such as @ref std::ios::cur).
    **/
    void seek(std::streamoff position, std::ios::seekdir anchor) const noexcept {
        inner_stream->seekg(position, anchor);
    }

    /**
    * @brief Reads the next UTF-8 rune from @ref inner_stream, moving forward by the number of bytes read.
    **/
    std::optional<std::string> read() noexcept {
        // Read first byte
        int first_byte_as_int = inner_stream->get();
        if (first_byte_as_int < 0) {
            return std::nullopt;
        }

        // Increment character counter
        char_counter++;

        // Single byte character performance optimization
        if (first_byte_as_int <= 127) {
            return std::string({ (char)first_byte_as_int });
        }

        // Get number of bytes in UTF8 character
        uint8_t sequence_length = get_utf8_sequence_length((uint8_t)first_byte_as_int);

        // Read remaining bytes (up to 3 more)
        std::string bytes((size_t)sequence_length, '\0');
        bytes[0] = (char)first_byte_as_int;
        inner_stream->read(&bytes[1], std::streamsize(sequence_length - 1));

        // Trim excess bytes
        bytes.resize(1 + inner_stream->gcount());
        return bytes;
    }
    /**
    * @brief Reads the next UTF-8 rune from @ref inner_stream, without moving forward.
    **/
    std::optional<std::string> peek() noexcept {
        std::streampos original_position = position();
        std::optional<std::string> next = read();
        if (next) {
            // Undo increment character counter
            char_counter--;
        }
        seek(original_position);
        return next;
    }
    /**
    * @brief If the next UTF-8 rune is the given option, moves forward by its number of bytes.
    **/
    bool read_one(std::string option) noexcept {
        if (peek() == option) {
            read();
            return true;
        }
        return false;
    }
    /**
    * @brief If the next UTF-8 rune is one of the given options, moves forward by its number of bytes and returns the option.
    **/
    std::optional<std::string> read_any(const std::set<std::string>& options) noexcept {
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
    std::optional<std::string> read_reverse() noexcept {
        // Read up to 4 bytes
        std::string bytes;
        bytes.reserve(4);
        for (size_t index = 0; index < 4; index++) {
            // Ensure last byte is available
            if (position() == 0) {
                return std::nullopt;
            }

            // Move before last byte
            seek(-1, std::ios::cur);

            // Peek next byte
            int next_as_int = inner_stream->peek();
            if (next_as_int < 0) {
                return std::nullopt;
            }

            // Increment character counter
            if (index == 0) {
                char_counter++;
            }

            // Append byte
            bytes += (char)next_as_int;

            // End if reached first byte
            if (is_utf8_first_byte((char)next_as_int)) {
                std::reverse(bytes.begin(), bytes.end());
                return bytes;
            }
        }

        // Never reached first byte
        return std::nullopt;
    }
    /**
    * @brief Reads the last UTF-8 rune from @ref inner_stream, without moving backward.
    **/
    std::optional<std::string> peek_reverse() noexcept {
        std::streampos original_position = position();
        std::optional<std::string> last = read_reverse();
        if (last) {
            // Undo increment character counter
            char_counter--;
        }
        seek(original_position);
        return last;
    }
    /**
    * @brief If the last UTF-8 rune is the given option, moves backward by its number of bytes.
    **/
    bool read_one_reverse(std::string option) noexcept {
        if (peek_reverse() == option) {
            read_reverse();
            return true;
        }
        return false;
    }
    /**
    * @brief If the last UTF-8 rune is one of the given options, moves backward by its number of bytes and returns the option.
    **/
    std::optional<std::string> read_any_reverse(const std::set<std::string>& options) noexcept {
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
    static constexpr bool is_utf8_first_byte(uint8_t byte) noexcept {
        return (byte & 0xC0) != 0x80;
    }
    /**
    * @brief Calculates the byte count of a UTF-8 rune from the bits in its first byte.
    *
    * @returns 1 or 2 or 3 or 4.
    **/
    static constexpr uint8_t get_utf8_sequence_length(uint8_t first_byte) noexcept {
        // https://codegolf.stackexchange.com/a/173577
        return ((first_byte - 160) >> (20 - (first_byte / 16))) + 2;
    }
};

}

#endif // JSONH_CPP_UTF8_READER_H
/*** End of inlined file: utf8_reader.hpp ***/

namespace jsonh_cpp {

/**
* @brief A reader that reads tokens from a UTF-8 input stream.
**/
class jsonh_reader : utf8_reader {
public:
    /**
    * @brief The options to use when reading JSONH.
    **/
    jsonh_reader_options options;
    /**
    * @brief The current recursion depth of the reader.
    **/
    int32_t depth;

    /**
    * @brief Constructs a reader that reads JSONH from a UTF-8 input stream.
    **/
    explicit jsonh_reader(std::unique_ptr<std::istream> stream, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : utf8_reader(std::move(stream)) {
        this->options = options;
        this->depth = 0;
    }
    /**
    * @brief Constructs a reader that reads JSONH from a UTF-8 string.
    **/
    explicit jsonh_reader(const std::string& string, jsonh_reader_options options = jsonh_reader_options()) noexcept
        : jsonh_reader(std::make_unique<std::istringstream>(string), options) {
    }

    /**
    * @brief Parses a single element from a UTF-8 input stream and deserializes it as @ref T.
    **/
    template <typename T>
    static std::expected<T, std::string> parse_element(std::unique_ptr<std::istream> stream, jsonh_reader_options options = jsonh_reader_options()) noexcept {
        return jsonh_reader(std::move(stream), options).parse_element<T>();
    }
    /**
    * @brief Parses a single element from a UTF-8 input stream.
    **/
    static std::expected<json_element, std::string> parse_element(std::unique_ptr<std::istream> stream, jsonh_reader_options options = jsonh_reader_options()) noexcept {
        return jsonh_reader(std::move(stream), options).parse_element();
    }
    /**
    * @brief Parses a single element from a UTF-8 string and deserializes it as @ref T.
    **/
    template <typename T>
    static std::expected<T, std::string> parse_element(const std::string& string, jsonh_reader_options options = jsonh_reader_options()) noexcept {
        return jsonh_reader(string, options).parse_element<T>();
    }
    /**
    * @brief Parses a single element from a UTF-8 string.
    **/
    static std::expected<json_element, std::string> parse_element(const std::string& string, jsonh_reader_options options = jsonh_reader_options()) noexcept {
        return jsonh_reader(string, options).parse_element();
    }

    /**
    * @brief Parses a single element from the reader and deserializes it as @ref T.
    **/
    template <typename T>
    std::expected<T, std::string> parse_element() noexcept {
        std::expected<json_element, std::string> element = parse_element();
        if (!element) {
            return std::unexpected(element.error());
        }
        return element.value().as<T>();
    }
    /**
    * @brief Parses a single element from the reader.
    **/
    std::expected<json_element, std::string> parse_element() noexcept {
        std::stack<json_element> current_elements;
        std::optional<std::string> current_property_name;

        auto submit_element = [&](const json_element& element) -> bool {
            // Root value
            if (current_elements.empty()) {
                return true;
            }
            // Array item
            if (!current_property_name) {
                current_elements.top().as_array().push_back(element);
                return false;
            }
            // Object property
            else {
                current_elements.top().as_object()[current_property_name.value()] = element;
                current_property_name.reset();
                return false;
            }
        };
        auto start_element = [&](const json_element& element) -> void {
            submit_element(element);
            current_elements.push(element);
        };
        auto parse_next_element = [&]() -> std::expected<json_element, std::string> {
            for (const std::expected<jsonh_token, std::string>& token_result : read_element()) {
                // Check error
                if (!token_result) {
                    return std::unexpected(token_result.error());
                }
                jsonh_token token = token_result.value();

                switch (token.json_type) {
                    // Null
                    case json_token_type::null: {
                        json_element element = json_element(nullptr);
                        if (submit_element(element)) {
                            return element;
                        }
                        break;
                    }
                    // True
                    case json_token_type::true_bool: {
                        json_element element = json_element(true);
                        if (submit_element(element)) {
                            return element;
                        }
                        break;
                    }
                    // False
                    case json_token_type::false_bool: {
                        json_element element = json_element(false);
                        if (submit_element(element)) {
                            return element;
                        }
                        break;
                    }
                    // String
                    case json_token_type::string: {
                        json_element element = json_element(token.value);
                        if (submit_element(element)) {
                            return element;
                        }
                        break;
                    }
                    // Number
                    case json_token_type::number: {
                        std::expected<double, std::string> result = jsonh_number_parser::parse(token.value);
                        if (!result) {
                            return std::unexpected(result.error());
                        }
                        json_element element = json_element(result.value());
                        if (submit_element(element)) {
                            return element;
                        }
                        break;
                    }
                    // Start Object
                    case json_token_type::start_object: {
                        json_element element = json_element::json_object_t();
                        start_element(element);
                        break;
                    }
                    // Start Array
                    case json_token_type::start_array: {
                        json_element element = json_element::json_array_t();
                        start_element(element);
                        break;
                    }
                    // End Object/Array
                    case json_token_type::end_object: case json_token_type::end_array: {
                        // Nested element
                        if (current_elements.size() > 1) {
                            current_elements.pop();
                        }
                        // Root element
                        else {
                            return current_elements.top();
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
            }

            // End of input
            return std::unexpected("Expected token, got end of input");
        };

        // Parse next element
        std::expected<json_element, std::string> next_element = parse_next_element();

        // Ensure exactly one element
        if (next_element) {
            if (options.parse_single_element) {
                for (const std::expected<jsonh_token, std::string>& token : read_end_of_elements()) {
                    if (!token) {
                        return std::unexpected(token.error());
                    }
                }
            }
        }

        return next_element;
    }
    /**
     * @brief Parses a single element as minified JSON from the reader.
     *
     * If @c include_comments is true, comments are included (block comments are escaped as @c / @c * and @c * @c /).
     *
     * If @c indent is not null, the output is pretty-printed with the given indentation.
     *
     * Note: The result is @b NOT safe to embed in HTML. To safely embed in HTML, you need to escape characters like less-than, greater-than and ampersand.
     */
    std::expected<std::string, std::string> parse_json(bool include_comments = false, std::optional<std::string> indent = std::nullopt) noexcept {
        auto parse_next_element_as_json = [&]() -> std::expected<std::string, std::string> {
            int64_t current_depth = 0;
            bool is_start_of_structure = true;
            bool is_property_value = false;

            std::string result_builder;

            for (const std::expected<jsonh_token, std::string>& token_result : read_element()) {
                // Check error
                if (!token_result) {
                    return std::unexpected(token_result.error());
                }
                jsonh_token token = token_result.value();

                // Add comments and indents
                if (!is_property_value) {
                    // Add comma before property/item
                    if ((token.json_type != json_token_type::none && token.json_type != json_token_type::comment) && current_depth > 0 && !is_start_of_structure) {
                        // Don't add trailing comma
                        if (token.json_type != json_token_type::end_object && token.json_type != json_token_type::end_array) {
                            result_builder.push_back(',');
                        }
                    }

                    // Apply indentation
                    if (indent) {
                        // Don't indent inside empty structures
                        if (!((token.json_type == json_token_type::end_object || token.json_type == json_token_type::end_array) && is_start_of_structure)) {
                            // Don't indent comment if not included
                            if (!(token.json_type == json_token_type::comment && !include_comments)) {
                                // Don't indent root elements
                                if (current_depth > 0) {
                                    // Add newline before element
                                    result_builder.push_back('\n');

                                    // Get current indent count
                                    int64_t indent_count = current_depth;
                                    if (token.json_type == json_token_type::end_object || token.json_type == json_token_type::end_array) {
                                        indent_count--;
                                    }

                                    // Add indent
                                    for (int64_t counter = 0; counter < indent_count; counter++) {
                                        result_builder.append(indent.value());
                                    }
                                }
                            }
                        }
                    }
                }
                // Track start of structure to avoid adding leading comma
                if (token.json_type != json_token_type::none && token.json_type != json_token_type::comment) {
                    is_start_of_structure = false;
                }
                if (token.json_type == json_token_type::start_object || token.json_type == json_token_type::start_array) {
                    is_start_of_structure = true;
                }

                switch (token.json_type) {
                    // Null
                    case json_token_type::null: {
                        result_builder.append("null");
                        if (current_depth == 0) {
                            return result_builder;
                        }
                        break;
                    }
                    // True
                    case json_token_type::true_bool: {
                        result_builder.append("true");
                        if (current_depth == 0) {
                            return result_builder;
                        }
                        break;
                    }
                    // False
                    case json_token_type::false_bool: {
                        result_builder.append("false");
                        if (current_depth == 0) {
                            return result_builder;
                        }
                        break;
                    }
                    // String
                    case json_token_type::string: {
                        result_builder.append(to_json_string(token.value));
                        if (current_depth == 0) {
                            return result_builder;
                        }
                        break;
                    }
                    // Number
                    case json_token_type::number: {
                        std::expected<double, std::string> result = jsonh_number_parser::parse(token.value);
                        if (!result) {
                            return std::unexpected(result.error());
                        }
                        result_builder.append(to_json_number(result.value()));
                        if (result_builder.ends_with(".0")) {
                            result_builder.erase(result_builder.length() - 2);
                        }
                        if (current_depth == 0) {
                            return result_builder;
                        }
                        break;
                    }
                    // Start Object
                    case json_token_type::start_object: {
                        result_builder.push_back('{');
                        current_depth++;
                        break;
                    }
                    // Start Array
                    case json_token_type::start_array: {
                        result_builder.push_back('[');
                        current_depth++;
                        break;
                    }
                    // End Object
                    case json_token_type::end_object: {
                        result_builder.push_back('}');
                        current_depth--;
                        if (current_depth == 0) {
                            return result_builder;
                        }
                        break;
                    }
                    // End Array
                    case json_token_type::end_array: {
                        result_builder.push_back(']');
                        current_depth--;
                        if (current_depth == 0) {
                            return result_builder;
                        }
                        break;
                    }
                    // Property Name
                    case json_token_type::property_name: {
                        result_builder.append(to_json_string(token.value));
                        result_builder.push_back(':');
                        if (indent) {
                            result_builder.push_back(' ');
                        }
                        break;
                    }
                    // Comment
                    case json_token_type::comment: {
                        if (include_comments) {
                            result_builder.append("/*");
                            std::string comment_value = token.value;
                            replace_all(comment_value, "/*", "/ *");
                            replace_all(comment_value, "*/", "* /");
                            result_builder.append(comment_value);
                            result_builder.append("*/");
                        }
                        break;
                    }
                    // Not implemented
                    default: {
                        return std::unexpected("Token type not implemented");
                    }
                }

                if (token.json_type != json_token_type::comment) {
                    is_property_value = token.json_type == json_token_type::property_name;
                }
            }

            // End of input
            return std::unexpected("Expected token, got end of input");
        };

        // Parse next element as JSON
        std::expected<std::string, std::string> next_element_as_json = parse_next_element_as_json();

        // Ensure exactly one element
        if (next_element_as_json) {
            if (options.parse_single_element) {
                for (const std::expected<jsonh_token, std::string>& token : read_end_of_elements()) {
                    if (!token) {
                        return std::unexpected(token.error());
                    }
                }
            }
        }

        return next_element_as_json;
    }
    /**
    * @brief Tries to find the given property name in the reader.
    *
    * For example, to find @c c:
    *
    * @code{.jsonh}
    * // Original position
    * {
    *   "a": "1",
    *   "b": {
    *     "c": "2"
    *   },
    *   "c": // Final position
    *        "3"
    * }
    * @endcode
    **/
    bool find_property_value(const std::string& property_name) noexcept {
        int64_t current_depth = 0;

        for (const std::expected<jsonh_token, std::string>& token_result : read_element()) {
            // Check error
            if (!token_result) {
                return false;
            }

            switch (token_result.value().json_type) {
                // Start structure
                case json_token_type::start_object: case json_token_type::start_array: {
                    current_depth++;
                    break;
                }
                // End structure
                case json_token_type::end_object: case json_token_type::end_array: {
                    current_depth--;
                    break;
                }
                // Property name
                case json_token_type::property_name: {
                    if (current_depth == 1 && token_result.value().value == property_name) {
                        // Path found
                        return true;
                    }
                    break;
                }
                // Other
                default: {
                    break;
                }
            }
        }

        // Path not found
        return false;
    }
    /**
    * @brief Reads whitespace and returns whether the reader contains another token.
    *
    * Note: This can be used to skip whitespace without skipping comments.
    **/
    bool has_token() noexcept {
        // Whitespace
        read_whitespace();

        // Peek char
        return !!peek();
    }
    /**
    * @brief Reads comments and whitespace and errors if the reader contains another element.
    **/
    std::generator<std::expected<jsonh_token, std::string>> read_end_of_elements() noexcept {
        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Peek char
        if (!!peek()) {
            co_yield(std::unexpected("Expected end of elements"));
            co_return;
        }
    }
    /**
    * @brief Reads a single element from the reader.
    **/
    std::generator<std::expected<jsonh_token, std::string>> read_element() noexcept {
        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Peek rune
        std::optional<std::string> next = peek();
        if (!next) {
            co_yield(std::unexpected("Expected token, got end of input"));
            co_return;
        }

        // Object
        if (next.value() == "{") {
            for (const std::expected<jsonh_token, std::string>& token : read_object()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }
        }
        // Array
        else if (next.value() == "[") {
            for (const std::expected<jsonh_token, std::string>& token : read_array()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }
        }
        // Primitive value (null, true, false, string, number)
        else {
            std::expected<jsonh_token, std::string> token = read_primitive_element();
            if (!token) {
                co_yield(std::unexpected(token.error()));
                co_return;
            }

            // Detect braceless object from property name
            for (const std::expected<jsonh_token, std::string>& token2 : read_braceless_object_or_end_of_primitive(token.value())) {
                if (!token2) {
                    co_yield(token2);
                    co_return;
                }
                co_yield(token2);
            }
        }
    }

private:
    /**
    * @brief Runes that cannot be used unescaped in quoteless strings.
    **/
    const std::set<std::string>& reserved_runes() { return options.supports_version(jsonh_version::v2) ? reserved_runes_v2 : reserved_runes_v1; }
    /**
    * @brief Runes that cannot be used unescaped in quoteless strings in JSONH V1.
    **/
    const std::set<std::string> reserved_runes_v1 = { "\\", ",", ":", "[", "]", "{", "}", "/", "#", "\"", "'" };
    /**
    * @brief Runes that cannot be used unescaped in quoteless strings in JSONH V2.
    **/
    const std::set<std::string> reserved_runes_v2 = { "\\", ",", ":", "[", "]", "{", "}", "/", "#", "\"", "'", "@" };
    /**
    * @brief Runes that are considered newlines.
    **/
    const std::set<std::string> newline_runes = { "\n", "\r", "\u2028", "\u2029" };
    /**
    * @brief Runes that are considered whitespace.
    **/
    const std::set<std::string> whitespace_runes = {
        "\u0020", "\u00A0", "\u1680", "\u2000", "\u2001", "\u2002", "\u2003", "\u2004", "\u2005",
        "\u2006", "\u2007", "\u2008", "\u2009", "\u200A", "\u202F", "\u205F", "\u3000", "\u2028",
        "\u2029", "\u0009", "\u000A", "\u000B", "\u000C", "\u000D", "\u0085",
    };

    std::generator<std::expected<jsonh_token, std::string>> read_object() noexcept {
        // Opening brace
        if (!read_one("{")) {
            // Braceless object
            for (const std::expected<jsonh_token, std::string>& token : read_braceless_object()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }
            co_return;
        }
        // Start of object
        co_yield(jsonh_token(json_token_type::start_object));
        depth++;

        // Check exceeded max depth
        if (depth > options.max_depth) {
            co_yield(std::unexpected("Exceeded max depth"));
            co_return;
        }

        while (true) {
            // Comments & whitespace
            for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }

            std::optional<std::string> next = peek();
            if (!next) {
                // End of incomplete object
                if (options.incomplete_inputs) {
                    depth--;
                    co_yield(jsonh_token(json_token_type::end_object));
                    co_return;
                }
                // Missing closing brace
                co_yield(std::unexpected("Expected `}` to end object, got end of input"));
                co_return;
            }

            // Closing brace
            if (next.value() == "}") {
                // End of object
                read();
                depth--;
                co_yield(jsonh_token(json_token_type::end_object));
                co_return;
            }
            // Property
            else {
                for (const std::expected<jsonh_token, std::string>& token : read_property()) {
                    if (!token) {
                        co_yield(token);
                        co_return;
                    }
                    co_yield(token);
                }
            }
        }
    }
    std::generator<std::expected<jsonh_token, std::string>> read_braceless_object(std::optional<std::vector<jsonh_token>> property_name_tokens = std::nullopt) noexcept {
        // Start of object
        co_yield(jsonh_token(json_token_type::start_object));
        depth++;

        // Check exceeded max depth
        if (depth > options.max_depth) {
            co_yield(std::unexpected("Exceeded max depth"));
            co_return;
        }

        // Initial tokens
        if (property_name_tokens) {
            for (const std::expected<jsonh_token, std::string>& token : read_property(property_name_tokens)) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }
        }

        while (true) {
            // Comments & whitespace
            for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }

            if (!peek()) {
                // End of braceless object
                depth--;
                co_yield(jsonh_token(json_token_type::end_object));
                co_return;
            }

            // Property
            for (const std::expected<jsonh_token, std::string>& token : read_property()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }
        }
    }
    std::generator<std::expected<jsonh_token, std::string>> read_braceless_object_or_end_of_primitive(const jsonh_token& primitive_token) {
        // Comments & whitespace
        std::optional<std::vector<jsonh_token>> property_name_tokens = {};
        for (const std::expected<jsonh_token, std::string>& comment_or_whitespace_token : read_comments_and_whitespace()) {
            if (!comment_or_whitespace_token) {
                co_yield(comment_or_whitespace_token);
                co_return;
            }
            if (!property_name_tokens) {
                property_name_tokens = std::vector<jsonh_token>();
            }
            property_name_tokens.value().push_back(comment_or_whitespace_token.value());
        }

        // Primitive
        if (!read_one(":")) {
            // Primitive
            co_yield(primitive_token);
            // Comments & whitespace
            if (property_name_tokens) {
                for (const jsonh_token& comment_or_whitespace_token : property_name_tokens.value()) {
                    co_yield(comment_or_whitespace_token);
                }
            }
            // End of primitive
            co_return;
        }

        // Property name
        if (!property_name_tokens) {
            property_name_tokens = std::vector<jsonh_token>();
        }
        property_name_tokens.value().emplace_back(jsonh_token(json_token_type::property_name, primitive_token.value));

        // Braceless object
        for (const std::expected<jsonh_token, std::string>& object_token : read_braceless_object(property_name_tokens)) {
            if (!object_token) {
                co_yield(object_token);
                co_return;
            }
            co_yield(object_token);
        }
    }
    std::generator<std::expected<jsonh_token, std::string>> read_property(std::optional<std::vector<jsonh_token>> property_name_tokens = std::nullopt) noexcept {
        // Property name
        if (property_name_tokens) {
            for (const jsonh_token& token : property_name_tokens.value()) {
                co_yield(token);
            }
        }
        else {
            for (const std::expected<jsonh_token, std::string>& token : read_property_name()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }
        }

        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Property value
        for (const std::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Optional comma
        read_one(",");
    }
    std::generator<std::expected<jsonh_token, std::string>> read_property_name() noexcept {
        // String
        std::expected<jsonh_token, std::string> string_result = read_string();
        if (!string_result) {
            co_yield(string_result);
            co_return;
        }
        jsonh_token string = string_result.value();

        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Colon
        if (!read_one(":")) {
            co_yield(std::unexpected("Expected `:` after property name in object"));
            co_return;
        }

        // End of property name
        co_yield(jsonh_token(json_token_type::property_name, string.value));
    }
    std::generator<std::expected<jsonh_token, std::string>> read_array() noexcept {
        // Opening bracket
        if (!read_one("[")) {
            co_yield(std::unexpected("Expected `[` to start array"));
            co_return;
        }
        // Start of array
        co_yield(jsonh_token(json_token_type::start_array));
        depth++;

        // Check exceeded max depth
        if (depth > options.max_depth) {
            co_yield(std::unexpected("Exceeded max depth"));
            co_return;
        }

        while (true) {
            // Comments & whitespace
            for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
                if (!token) {
                    co_yield(token);
                    co_return;
                }
                co_yield(token);
            }

            std::optional<std::string> next = peek();
            if (!next) {
                // End of incomplete array
                if (options.incomplete_inputs) {
                    depth--;
                    co_yield(jsonh_token(json_token_type::end_array));
                    co_return;
                }
                // Missing closing bracket
                co_yield(std::unexpected("Expected `]` to end array, got end of input"));
                co_return;
            }

            // Closing bracket
            if (next.value() == "]") {
                // End of array
                read();
                depth--;
                co_yield(jsonh_token(json_token_type::end_array));
                co_return;
            }
            // Item
            else {
                for (const std::expected<jsonh_token, std::string>& token : read_item()) {
                    if (!token) {
                        co_yield(token);
                        co_return;
                    }
                    co_yield(token);
                }
            }
        }
    }
    std::generator<std::expected<jsonh_token, std::string>> read_item() noexcept {
        // Element
        for (const std::expected<jsonh_token, std::string>& token : read_element()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Comments & whitespace
        for (const std::expected<jsonh_token, std::string>& token : read_comments_and_whitespace()) {
            if (!token) {
                co_yield(token);
                co_return;
            }
            co_yield(token);
        }

        // Optional comma
        read_one(",");
    }
    std::expected<jsonh_token, std::string> read_string() noexcept {
        // Verbatim
        bool is_verbatim = false;
        if (options.supports_version(jsonh_version::v2) && read_one("@")) {
            is_verbatim = true;

            // Ensure string immediately follows verbatim symbol
            std::optional<std::string> next = peek();
            if (!next || next.value() == "#" || next.value() == "/" || whitespace_runes.contains(next.value())) {
                return std::unexpected("Expected string to immediately follow verbatim symbol");
            }
        }

        // Start quote
        std::optional<std::string> start_quote = read_any({ "\"", "'" });
        if (!start_quote) {
            return read_quoteless_string("", is_verbatim);
        }
        char start_quote_char = start_quote.value()[0];

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
        std::string string_builder;

        while (true) {
            std::optional<std::string> next = read();
            if (!next) {
                return std::unexpected("Expected end of string, got end of input");
            }

            // Partial end quote was actually part of string
            if (next != start_quote) {
                string_builder.append(end_quote_counter, start_quote_char);
                end_quote_counter = 0;
            }

            // End quote
            if (next.value() == start_quote) {
                end_quote_counter++;
                if (end_quote_counter == start_quote_counter) {
                    break;
                }
            }
            // Escape sequence
            else if (next.value() == "\\") {
                if (is_verbatim) {
                    string_builder += next.value();
                }
                else {
                    std::expected<std::string, std::string> escape_sequence_result = read_escape_sequence();
                    if (!escape_sequence_result) {
                        return std::unexpected(escape_sequence_result.error());
                    }
                    string_builder += escape_sequence_result.value();
                }
            }
            // Literal character
            else {
                string_builder += next.value();
            }
        }

        // Condition: skip remaining steps unless started with multiple quotes
        if (start_quote_counter > 1) {
            // Get chars from string builder
            std::vector<std::string> string_builder_chars = {};
            utf8_reader string_builder_chars_reader(string_builder);
            while (true) {
                std::optional<std::string> next = string_builder_chars_reader.read();
                if (!next) {
                    break;
                }
                string_builder_chars.push_back(next.value());
            }

            // Pass 1: count leading whitespace -> newline
            bool has_leading_whitespace_newline = false;
            size_t leading_whitespace_newline_counter = 0;
            for (size_t index = 0; index < string_builder_chars.size(); index++) {
                std::string next = string_builder_chars[index];

                // Newline
                if (newline_runes.contains(next)) {
                    // Join CR LF
                    if (next == "\r" && index + 1 < string_builder_chars.size() && string_builder_chars[index + 1] == "\n") {
                        index++;
                    }

                    has_leading_whitespace_newline = true;
                    leading_whitespace_newline_counter = index + 1;
                    break;
                }
                // Non-whitespace
                else if (!whitespace_runes.contains(next)) {
                    break;
                }
            }

            // Condition: skip remaining steps if pass 1 failed
            if (has_leading_whitespace_newline) {
                // Pass 2: count trailing newline -> whitespace
                bool has_trailing_newline_whitespace = false;
                size_t last_newline_index = 0;
                size_t trailing_whitespace_counter = 0;
                for (size_t index = 0; index < string_builder_chars.size(); index++) {
                    std::string next = string_builder_chars[index];

                    // Newline
                    if (newline_runes.contains(next)) {
                        has_trailing_newline_whitespace = true;
                        last_newline_index = index;
                        trailing_whitespace_counter = 0;

                        // Join CR LF
                        if (next == "\r" && index + 1 < string_builder_chars.size() && string_builder_chars[index + 1] == "\n") {
                            index++;
                        }
                    }
                    // Whitespace
                    else if (whitespace_runes.contains(next)) {
                        trailing_whitespace_counter++;
                    }
                    // Non-whitespace
                    else {
                        has_trailing_newline_whitespace = false;
                        trailing_whitespace_counter = 0;
                    }
                }

                // Condition: skip remaining steps if pass 2 failed
                if (has_trailing_newline_whitespace) {
                    // Pass 3: strip trailing newline -> whitespace
                    string_builder_chars.erase(string_builder_chars.begin() + last_newline_index, string_builder_chars.begin() + string_builder_chars.size());

                    // Pass 4: strip leading whitespace -> newline
                    string_builder_chars.erase(string_builder_chars.begin(), string_builder_chars.begin() + leading_whitespace_newline_counter);

                    // Condition: skip remaining steps if no trailing whitespace
                    if (trailing_whitespace_counter > 0) {
                        // Pass 5: strip line-leading whitespace
                        bool is_line_leading_whitespace = true;
                        size_t line_leading_whitespace_counter = 0;
                        for (size_t index = 0; index < string_builder_chars.size(); index++) { // Wrapping add
                            std::string next = string_builder_chars[index];

                            // Newline
                            if (newline_runes.contains(next)) {
                                is_line_leading_whitespace = true;
                                line_leading_whitespace_counter = 0;
                            }
                            // Whitespace
                            else if (whitespace_runes.contains(next)) {
                                if (is_line_leading_whitespace) {
                                    // Increment line-leading whitespace
                                    line_leading_whitespace_counter++;

                                    // Maximum line-leading whitespace reached
                                    if (line_leading_whitespace_counter == trailing_whitespace_counter) {
                                        // Remove line-leading whitespace
                                        string_builder_chars.erase(string_builder_chars.begin() + index + 1 - line_leading_whitespace_counter, string_builder_chars.begin() + index + 1);
                                        index -= line_leading_whitespace_counter; // Wrapping sub
                                        // Exit line-leading whitespace
                                        is_line_leading_whitespace = false;
                                    }
                                }
                            }
                            // Non-whitespace
                            else {
                                if (is_line_leading_whitespace) {
                                    // Remove partial line-leading whitespace
                                    string_builder_chars.erase(string_builder_chars.begin() + index - line_leading_whitespace_counter, string_builder_chars.begin() + index);
                                    index -= line_leading_whitespace_counter; // Wrapping sub
                                    // Exit line-leading whitespace
                                    is_line_leading_whitespace = false;
                                }
                            }
                        }
                    }
                }
            }

            // Get string builder from chars
            string_builder.clear();
            for (const std::string& c : string_builder_chars) {
                string_builder += c;
            }
        }

        // End of string
        return jsonh_token(json_token_type::string, string_builder);
    }
    std::expected<jsonh_token, std::string> read_quoteless_string(const std::string& initial_chars = "", bool is_verbatim = false) noexcept {
        bool is_named_literal_possible = !is_verbatim;

        // Read quoteless string
        std::string string_builder = initial_chars;

        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Escape sequence
            if (next.value() == "\\") {
                read();
                if (is_verbatim) {
                    string_builder += next.value();
                }
                else {
                    std::expected<std::string, std::string> escape_sequence_result = read_escape_sequence();
                    if (!escape_sequence_result) {
                        return std::unexpected(escape_sequence_result.error());
                    }
                    string_builder += escape_sequence_result.value();
                }
                is_named_literal_possible = false;
            }
            // End on reserved character
            else if (reserved_runes().contains(next.value())) {
                break;
            }
            // End on newline
            else if (newline_runes.contains(next.value())) {
                break;
            }
            // Literal character
            else {
                read();
                string_builder += next.value();
            }
        }

        // Ensure not empty
        if (string_builder.empty()) {
            return std::unexpected("Empty quoteless string");
        }

        // Trim leading whitespace
        utf8_reader string_builder_reader(string_builder);
        while (true) {
            std::streampos original_position = string_builder_reader.position();
            std::optional<std::string> next = string_builder_reader.read();

            // Non-whitespace
            if (!next || !whitespace_runes.contains(next.value())) {
                string_builder.erase(0, (size_t)original_position);
                break;
            }
        }
        // Trim trailing whitespace
        utf8_reader string_builder_reader2(string_builder);
        string_builder_reader2.seek(0, std::ios_base::end);
        while (true) {
            std::streampos original_position = string_builder_reader2.position();
            std::optional<std::string> last = string_builder_reader2.read_reverse();

            // Non-whitespace
            if (!last || !whitespace_runes.contains(last.value())) {
                string_builder.erase((size_t)original_position);
                break;
            }
        }

        // Match named literal
        if (is_named_literal_possible) {
            if (string_builder == "null") {
                return jsonh_token(json_token_type::null, "null");
            }
            else if (string_builder == "true") {
                return jsonh_token(json_token_type::true_bool, "true");
            }
            else if (string_builder == "false") {
                return jsonh_token(json_token_type::false_bool, "false");
            }
        }

        // End quoteless string
        return jsonh_token(json_token_type::string, string_builder);
    }
    bool detect_quoteless_string(std::string& whitespace_builder) {
        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Newline
            if (newline_runes.contains(next.value())) {
                // Quoteless strings cannot contain unescaped newlines
                return false;
            }

            // End of whitespace
            if (!whitespace_runes.contains(next.value())) {
                break;
            }

            // Whitespace
            whitespace_builder += next.value();
            read();
        }

        // Found quoteless string if found backslash or non-reserved char
        std::optional<std::string> next_char = peek();
        return next_char && (next_char.value() == "\\" || !reserved_runes().contains(next_char.value()));
    }
    std::expected<jsonh_token, std::string> read_number(std::string& number_builder) noexcept {
        // Read sign
        std::optional<std::string> sign = read_any({ "-", "+" });
        if (sign) {
            number_builder += sign.value();
        }

        // Read base
        std::string base_digits = "0123456789";
        bool has_base_specifier = false;
        bool has_leading_zero = false;
        if (read_one("0")) {
            number_builder += '0';
            has_leading_zero = true;

            std::optional<std::string> hex_base_char = read_any({ "x", "X" });
            if (hex_base_char) {
                number_builder += hex_base_char.value();
                base_digits = "0123456789abcdef";
                has_base_specifier = true;
                has_leading_zero = false;
            }
            else {
                std::optional<std::string> binary_base_char = read_any({ "b", "B" });
                if (binary_base_char) {
                    number_builder += binary_base_char.value();
                    base_digits = "01";
                    has_base_specifier = true;
                    has_leading_zero = false;
                }
                else {
                    std::optional<std::string> octal_base_char = read_any({ "o", "O" });
                    if (octal_base_char) {
                        number_builder += octal_base_char.value();
                        base_digits = "01234567";
                        has_base_specifier = true;
                        has_leading_zero = false;
                    }
                }
            }
        }

        // Read main number
        std::expected<void, std::string> main_result = read_number_no_exponent(number_builder, base_digits, has_base_specifier, has_leading_zero);
        if (!main_result) {
            return std::unexpected(main_result.error());
        }

        // Possible hexadecimal exponent
        if (number_builder.back() == 'e' || number_builder.back() == 'E') {
            // Read sign (mandatory)
            std::optional<std::string> exponent_sign = read_any({ "-", "+" });
            if (exponent_sign) {
                number_builder += exponent_sign.value();

                // Missing digit between base specifier and exponent (e.g. `0xe+`)
                if (has_base_specifier && number_builder.length() == 4) {
                    return std::unexpected("Missing digit between base specifier and exponent");
                }

                // Read exponent number
                std::expected<void, std::string> exponent_result = read_number_no_exponent(number_builder, base_digits);
                if (!exponent_result) {
                    return std::unexpected(exponent_result.error());
                }
            }
        }
        // Exponent
        else {
            std::optional<std::string> exponent_char = read_any({ "e", "E" });
            if (exponent_char) {
                number_builder += exponent_char.value();

                // Read sign
                std::optional<std::string> exponent_sign = read_any({ "-", "+" });
                if (exponent_sign) {
                    number_builder += exponent_sign.value();
                }

                // Read exponent number
                std::expected<void, std::string> exponent_result = read_number_no_exponent(number_builder, base_digits);
                if (!exponent_result) {
                    return std::unexpected(exponent_result.error());
                }
            }
        }

        // End of number
        return jsonh_token(json_token_type::number, number_builder);
    }
    std::expected<void, std::string> read_number_no_exponent(std::string& number_builder, std::string_view base_digits, bool has_base_specifier = false, bool has_leading_zero = false) noexcept {
        // Leading underscore
        if (!has_base_specifier && !has_leading_zero && peek() == "_") {
            return std::unexpected("Leading `_` in number");
        }

        bool is_fraction = false;
        bool is_empty = true;

        // Leading zero (not base specifier)
        if (has_leading_zero) {
            is_empty = false;
        }

        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Digit
            if (base_digits.find(to_ascii_lower(next.value().data())) != std::string::npos) {
                read();
                number_builder += next.value();
                is_empty = false;
            }
            // Dot
            else if (next.value() == ".") {
                // Disallow dot following underscore
                if (number_builder.length() >= 1 && number_builder.back() == '_') {
                    return std::unexpected("`.` must not follow `_` in number");
                }

                read();
                number_builder += next.value();
                is_empty = false;

                // Duplicate dot
                if (is_fraction) {
                    return std::unexpected("Duplicate `.` in number");
                }
                is_fraction = true;
            }
            // Underscore
            else if (next.value() == "_") {
                // Disallow underscore following dot
                if (number_builder.length() >= 1 && number_builder.back() == '.') {
                    return std::unexpected("`_` must not follow `.` in number");
                }

                read();
                number_builder += next.value();
                is_empty = false;
            }
            // Other
            else {
                break;
            }
        }

        // Ensure not empty
        if (is_empty) {
            return std::unexpected("Empty number");
        }

        // Ensure at least one digit
        if (number_builder.find_first_not_of(".-+_") == std::string::npos) {
            return std::unexpected("Number must have at least one digit");
        }

        // Trailing underscore
        if (number_builder.ends_with('_')) {
            return std::unexpected("Trailing `_` in number");
        }

        // End of number
        return std::expected<void, std::string>(); // Success
    }
    std::expected<jsonh_token, std::string> read_number_or_quoteless_string() noexcept {
        // Read number
        std::string number_builder;
        std::expected<jsonh_token, std::string> number = read_number(number_builder);
        if (number) {
            // Try read quoteless string starting with number
            std::string whitespace_chars;
            if (detect_quoteless_string(whitespace_chars)) {
                return read_quoteless_string(number.value().value + whitespace_chars);
            }
            // Otherwise, accept number
            else {
                return number;
            }
        }
        // Read quoteless string starting with malformed number
        else {
            return read_quoteless_string(number_builder);
        }
    }
    std::expected<jsonh_token, std::string> read_primitive_element() noexcept {
        // Peek rune
        std::optional<std::string> next = peek();
        if (!next) {
            return std::unexpected("Expected primitive element, got end of input");
        }

        // Number
        if (((next.value() >= "0" && next.value() <= "9") || (next.value() == "-" || next.value() == "+") || next.value() == ".")) {
            return read_number_or_quoteless_string();
        }
        // String
        else if (next.value() == "\"" || next.value() == "'" || (options.supports_version(jsonh_version::v2) && next.value() == "@")) {
            return read_string();
        }
        // Quoteless string (or named literal)
        else {
            return read_quoteless_string();
        }
    }
    std::generator<std::expected<jsonh_token, std::string>> read_comments_and_whitespace() noexcept {
        while (true) {
            // Whitespace
            read_whitespace();

            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                break;
            }

            // Comment
            if (next.value() == "#" || next.value() == "/") {
                std::expected<jsonh_token, std::string> comment = read_comment();
                if (!comment) {
                    co_yield(comment);
                    co_return;
                }
                co_yield(comment);
            }
            // End of comments
            else {
                break;
            }
        }
    }
    std::expected<jsonh_token, std::string> read_comment() noexcept {
        bool block_comment = false;
        int32_t start_nest_counter = 0;

        // Hash-style comment
        if (read_one("#")) {
        }
        else if (read_one("/")) {
            // Line-style comment
            if (read_one("/")) {
            }
            // Block-style comment
            else if (read_one("*")) {
                block_comment = true;
            }
            // Nestable block-style comment
            else if (options.supports_version(jsonh_version::v2) && peek() == "=") {
                block_comment = true;
                while (read_one("=")) {
                    start_nest_counter++;
                }
                if (!read_one("*")) {
                    return std::unexpected("Expected `*` after start of nesting block comment");
                }
            }
            else {
                return std::unexpected("Unexpected `/`");
            }
        }
        else {
            return std::unexpected("Unexpected character");
        }

        // Read comment
        std::string comment_builder;

        while (true) {
            // Read rune
            std::optional<std::string> next = read();

            if (block_comment) {
                // Error
                if (!next) {
                    return std::unexpected("Expected end of block comment, got end of input");
                }

                // End of block comment
                if (next.value() == "*") {
                    // End of nestable block comment
                    if (options.supports_version(jsonh_version::v2)) {
                        // Count nests
                        int32_t end_nest_counter = 0;
                        while (end_nest_counter < start_nest_counter && read_one("=")) {
                            end_nest_counter++;
                        }
                        // Partial end nestable block comment was actually part of comment
                        if (end_nest_counter < start_nest_counter || peek() != "/") {
                            comment_builder += "*";
                            for (; end_nest_counter > 0; end_nest_counter--) {
                                comment_builder += "=";
                            }
                            continue;
                        }
                    }

                    // End of block comment
                    if (read_one("/")) {
                        return jsonh_token(json_token_type::comment, comment_builder);
                    }
                }
            }
            else {
                // End of line comment
                if (!next || newline_runes.contains(next.value())) {
                    return jsonh_token(json_token_type::comment, comment_builder);
                }
            }

            // Comment char
            comment_builder += next.value();
        }
    }
    void read_whitespace() noexcept {
        while (true) {
            // Peek rune
            std::optional<std::string> next = peek();
            if (!next) {
                return;
            }

            // Whitespace
            if (whitespace_runes.contains(next.value())) {
                read();
            }
            // End of whitespace
            else {
                return;
            }
        }
    }
    template <size_t LENGTH>
    std::expected<uint32_t, std::string> read_hex_sequence() noexcept {
        static_assert(LENGTH <= 8);

        uint32_t value = 0;

        for (size_t index = 0; index < LENGTH; index++) {
            std::optional<std::string> next = read();

            // Hex digit
            if (next && ((next >= "0" && next <= "9") || (next >= "A" && next <= "F") || (next >= "a" && next <= "f"))) {
                // Get hex digit
                char digit = next.value()[0];
                // Convert hex digit to integer
                uint32_t integer =
                    (digit >= 'A' && digit <= 'F') ? digit - 'A' + 10 :
                    (digit >= 'a' && digit <= 'f') ? digit - 'a' + 10 :
                    digit - '0';
                // Aggregate digit into value
                value = (value * 16) + integer;
            }
            // Unexpected char
            else {
                return std::unexpected("Incorrect number of hexadecimal digits in unicode escape sequence");
            }
        }

        // Return aggregated value
        return value;
    }
    std::expected<std::string, std::string> read_escape_sequence(std::optional<uint32_t> high_surrogate = std::nullopt) noexcept {
        std::optional<std::string> escape_char = read();
        if (!escape_char) {
            return std::unexpected("Expected escape sequence, got end of input");
        }

        // Ensure high surrogates are completed
        if (high_surrogate && escape_char.value() != "u" && escape_char.value() != "x" && escape_char.value() != "U") {
            return std::unexpected("Expected low surrogate after high surrogate");
        }

        // Reverse solidus
        if (escape_char.value() == "\\") {
            return "\\";
        }
        // Backspace
        else if (escape_char.value() == "b") {
            return "\b";
        }
        // Form feed
        else if (escape_char.value() == "f") {
            return "\f";
        }
        // Newline
        else if (escape_char.value() == "n") {
            return "\n";
        }
        // Carriage return
        else if (escape_char.value() == "r") {
            return "\r";
        }
        // Tab
        else if (escape_char.value() == "t") {
            return "\t";
        }
        // Vertical tab
        else if (escape_char.value() == "v") {
            return "\v";
        }
        // Null
        else if (escape_char.value() == "0") {
            return "\0";
        }
        // Alert
        else if (escape_char.value() == "a") {
            return "\a";
        }
        // Escape
        else if (escape_char.value() == "e") {
            return "\u001b";
        }
        // Unicode hex sequence
        else if (escape_char.value() == "u") {
            return read_hex_escape_sequence<4>(high_surrogate);
        }
        // Short unicode hex sequence
        else if (escape_char.value() == "x") {
            return read_hex_escape_sequence<2>(high_surrogate);
        }
        // Long unicode hex sequence
        else if (escape_char.value() == "U") {
            return read_hex_escape_sequence<8>(high_surrogate);
        }
        // Escaped newline
        else if (newline_runes.contains(escape_char.value())) {
            // Join CR LF
            if (escape_char == "\r") {
                read_one("\n");
            }
            return "";
        }
        // Other
        else {
            return escape_char.value();
        }
    }
    template <size_t LENGTH>
    std::expected<std::string, std::string> read_hex_escape_sequence(std::optional<uint32_t> high_surrogate) noexcept {
        std::expected<uint32_t, std::string> code_point = read_hex_sequence<LENGTH>();
        if (!code_point) {
            return std::unexpected(code_point.error());
        }

        // Low surrogate
        if (high_surrogate) {
            std::expected<uint32_t, std::string> combined = utf16_surrogates_to_code_point(high_surrogate.value(), code_point.value());
            if (!combined) {
                return std::unexpected(combined.error());
            }
            return code_point_to_utf8(combined.value());
        }
        else {
            // High surrogate followed by low surrogate
            if (is_utf16_high_surrogate(code_point.value()) && read_one("\\")) {
                return read_escape_sequence(code_point.value());
            }
            // Standalone character
            else {
                return code_point_to_utf8(code_point.value());
            }
        }
    }
    static std::expected<std::string, std::string> code_point_to_utf8(uint32_t code_point) noexcept {
        // Invalid surrogate
        if (code_point >= 0xD800 && code_point <= 0xDFFF) {
            return std::unexpected("Invalid code point (surrogate half)");
        }
        // 1-byte UTF-8
        else if (code_point <= 0x7F) {
            return std::string({
                (char)code_point,
            });
        }
        // 2-byte UTF-8
        else if (code_point <= 0x7FF) {
            return std::string({
                (char)(0xC0 | (code_point >> 6)),
                (char)(0x80 | (code_point & 0x3F)),
            });
        }
        // 3-byte UTF-8
        else if (code_point <= 0xFFFF) {
            return std::string({
                (char)(0xE0 | (code_point >> 12)),
                (char)(0x80 | ((code_point >> 6) & 0x3F)),
                (char)(0x80 | (code_point & 0x3F)),
            });
        }
        // 4-byte UTF-8
        else if (code_point <= 0x10FFFF) {
            return std::string({
                (char)(0xF0 | (code_point >> 18)),
                (char)(0x80 | ((code_point >> 12) & 0x3F)),
                (char)(0x80 | ((code_point >> 6) & 0x3F)),
                (char)(0x80 | (code_point & 0x3F)),
            });
        }
        // Invalid UTF-8
        else {
            return std::unexpected("Invalid code point (out of range)");
        }
    }
    static std::expected<uint32_t, std::string> utf16_surrogates_to_code_point(uint32_t high_surrogate, uint32_t low_surrogate) noexcept {
        if (!is_utf16_high_surrogate(high_surrogate)) {
            return std::unexpected("High surrogate out of range");
        }
        if (!is_utf16_low_surrogate(low_surrogate)) {
            return std::unexpected("Low surrogate out of range");
        }
        return 0x10000 + (((high_surrogate - 0xD800) << 10) | (low_surrogate - 0xDC00));
    }
    static constexpr bool is_utf16_high_surrogate(uint32_t code_point) noexcept {
        return code_point >= 0xD800 && code_point <= 0xDBFF;
    }
    static constexpr bool is_utf16_low_surrogate(uint32_t code_point) noexcept {
        return code_point >= 0xDC00 && code_point <= 0xDFFF;
    }
    static std::string to_ascii_lower(const char* string) noexcept {
        std::string result(string);
        for (char& next : result) {
            if (next <= 'Z' && next >= 'A') {
                next -= ('Z' - 'z');
            }
        }
        return result;
    }
    static void replace_all(std::string& string, const std::string& search, const std::string& replace) {
        for (size_t pos = 0; ; pos += replace.length()) {
            // Locate the substring to replace
            pos = string.find(search, pos);
            if (pos == std::string::npos) {
                break;
            }
            // Replace by erasing and inserting
            string.erase(pos, search.length());
            string.insert(pos, replace);
        }
    }
    std::string to_json_string(const std::string& string) {
        std::string result = "\"";

        for (char character : string) {
            switch (character) {
                case '"':
                    result += "\\\"";
                    break;
                case '\\':
                    result += "\\\\";
                    break;
                case '\b':
                    result += "\\b";
                    break;
                case '\f':
                    result += "\\f";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    result += character;
            }
        }

        result += '"';
        return result;
    }
    std::string to_json_number(double value) {
        std::ostringstream stream;
        stream << std::defaultfloat << value;
        return stream.str();
    }
};

}

#endif // JSONH_CPP_JSONH_READER_H
/*** End of inlined file: jsonh_reader.hpp ***/

#endif // JSONH_CPP_JSONH_CPP_H
