#pragma once

#include <string> // for std::string
#include <cmath> // for pow
#include "martinmoene/expected.hpp" // for nonstd::expected (std::expected backport)

namespace jsonh_cpp {

/// <summary>
/// Methods for parsing JSONH numbers (long long / long double).<br/>
/// Unlike jsonh_reader.read_element, minimal validation is done here. Ensure the input is valid.
/// </summary>
class jsonh_number_parser final {
public:
    /// <summary>
    /// Converts a JSONH number to a base-10 real.
    /// For example:<br/>
    /// Input: <c>+5.2e3.0</c><br/>
    /// Output: <c>5200</c>
    /// </summary>
    static nonstd::expected<long double, std::string> parse(std::string jsonh_number) noexcept {
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
        nonstd::expected<long double, std::string> number = parse_fractional_number_with_exponent(digits, base_digits);
        if (!number) {
            return nonstd::unexpected<std::string>(number.error());
        }

        // Apply sign
        if (sign != 1) {
            number.value() *= sign;
        }
        return number;
    }

private:
    /// <summary>
    /// Converts a fractional number with an exponent (e.g. <c>12.3e4.5</c>) from the given base (e.g. <c>01234567</c>) to a base-10 real.
    /// </summary>
    static nonstd::expected<long double, std::string> parse_fractional_number_with_exponent(std::string_view digits, std::string_view base_digits) noexcept {
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
        // If no exponent then normalize real
        if (exponent_index == std::string::npos) {
            return parse_fractional_number(digits, base_digits);
        }

        // Get mantissa and exponent
        std::string_view mantissa_part = digits.substr(0, exponent_index);
        std::string_view exponent_part = digits.substr(exponent_index + 1);

        // Parse mantissa and exponent
        nonstd::expected<long double, std::string> mantissa = parse_fractional_number(mantissa_part, base_digits);
        if (!mantissa) {
            return nonstd::unexpected<std::string>(mantissa.error());
        }
        nonstd::expected<long double, std::string> exponent = parse_fractional_number(exponent_part, base_digits);
        if (!exponent) {
            return nonstd::unexpected<std::string>(exponent.error());
        }

        // Multiply mantissa by 10 ^ exponent
        return mantissa.value() * pow(10, exponent.value());
    }
    /// <summary>
    /// Converts a fractional number (e.g. <c>123.45</c>) from the given base (e.g. <c>01234567</c>) to a base-10 real.
    /// </summary>
    static nonstd::expected<long double, std::string> parse_fractional_number(std::string_view digits, std::string_view base_digits) noexcept {
        // Optimization for base-10 digits
        if (base_digits == "0123456789") {
            return std::stold(std::string(digits));
        }

        // Find dot
        size_t dot_index = digits.find('.');
        // If no dot then normalize integer
        if (dot_index == std::string::npos) {
            nonstd::expected<long long, std::string> integer = parse_whole_number(digits, base_digits);
            if (!integer) {
                return nonstd::unexpected<std::string>(integer.error());
            }
            return (long double)integer.value();
        }

        // Get parts of number
        std::string_view whole_part = digits.substr(0, dot_index);
        std::string_view fraction_part = digits.substr(dot_index + 1);

        // Parse parts of number
        nonstd::expected<long long, std::string> whole = parse_whole_number(whole_part, base_digits);
        if (!whole) {
            return nonstd::unexpected<std::string>(whole.error());
        }
        nonstd::expected<long long, std::string> fraction = parse_whole_number(fraction_part, base_digits);
        if (!fraction) {
            return nonstd::unexpected<std::string>(fraction.error());
        }

        // Combine whole and fraction
        return std::stold(std::to_string(whole.value()) + "." + std::to_string(fraction.value()));
    }
    /// <summary>
    /// Converts a whole number (e.g. <c>12345</c>) from the given base (e.g. <c>01234567</c>) to a base-10 integer.
    /// </summary>
    static nonstd::expected<long long, std::string> parse_whole_number(std::string_view digits, std::string_view base_digits) noexcept {
        // Optimization for base-10 digits
        if (base_digits == "0123456789") {
            return std::stoll(digits.data());
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
        long long integer = 0;
        for (size_t index = 0; index < digits.size(); index++) {
            // Get current digit
            char digit_char = digits[index];
            size_t digit_int = base_digits.find((char)std::tolower(digit_char));

            // Ensure digit is valid
            if (digit_int == std::string::npos) {
                return nonstd::unexpected<std::string>("Invalid digit");
            }

            // Get magnitude of current digit column
            size_t column_number = digits.size() - 1 - index;
            long long column_magnitude = (long long)pow(base_digits.size(), column_number);

            // Add value of column
            integer += (long long)digit_int * column_magnitude;
        }

        // Apply sign
        if (sign != 1) {
            integer *= sign;
        }
        return integer;
    }
};

}