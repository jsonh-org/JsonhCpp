#pragma once

#include <string>
#include <string_view>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <ios>
#include <sstream>
#include "martinmoene/expected.hpp"

namespace jsonh_cpp {

/**
* @brief Methods for parsing JSONH numbers (long double).
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
    /**
    * @brief Converts a fractional number with an exponent (e.g. @c 12.3e4.5) from the given base (e.g. @c 01234567) to a base-10 real.
    **/
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

        // If no exponent then parse real
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
    /**
    * @brief Converts a fractional number (e.g. @c 123.45) from the given base (e.g. @c 01234567) to a base-10 real.
    **/
    static nonstd::expected<long double, std::string> parse_fractional_number(std::string_view digits, std::string_view base_digits) noexcept {
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
        nonstd::expected<long double, std::string> whole = parse_whole_number(whole_part, base_digits);
        if (!whole) {
            return nonstd::unexpected<std::string>(whole.error());
        }
        nonstd::expected<long double, std::string> fraction = parse_whole_number(fraction_part, base_digits);
        if (!fraction) {
            return nonstd::unexpected<std::string>(fraction.error());
        }

        // Get fraction leading zeroes
        size_t fraction_leading_zeroes_count = 0;
        for (size_t index = 0; index < fraction_part.size(); index++) {
            if (fraction_part[index] == '0') {
                fraction_leading_zeroes_count++;
            }
            else {
                break;
            }
        }
        std::string fraction_leading_zeroes = std::string(fraction_leading_zeroes_count, '0');

        // Combine whole and fraction
        std::string whole_digits = number_to_string(whole.value());
        std::string fraction_digits = number_to_string(fraction.value());
        std::string combined = whole_digits + "." + fraction_leading_zeroes + fraction_digits;
        return string_to_number(combined);
    }
    /**
    * @brief Converts a whole number (e.g. @c 12345) from the given base (e.g. @c 01234567) to a base-10 integer.
    **/
    static nonstd::expected<long double, std::string> parse_whole_number(std::string_view digits, std::string_view base_digits) noexcept {
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
        long double integer = 0;
        for (size_t index = 0; index < digits.size(); index++) {
            // Get current digit
            char digit_char = digits[index];
            size_t digit_int = base_digits.find((char)std::tolower(digit_char));

            // Ensure digit is valid
            if (digit_int == std::string::npos) {
                return nonstd::unexpected<std::string>("Invalid digit");
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
    static std::string number_to_string(long double value) {
        std::ostringstream oss;
        oss << std::noshowpoint << value;
        return oss.str();
    }
    static long double string_to_number(std::string value) {
        return std::strtold(value.c_str(), nullptr);
    }
};

}