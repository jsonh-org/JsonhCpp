#ifndef JSONH_CPP_JSON_ELEMENT_H
#define JSONH_CPP_JSON_ELEMENT_H

#include <cstdint>
#include <vector>
#include <map>
#include <variant>
#include <string>
#include <utility>
#include <concepts>
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
    null,
    /**
    * @brief A boolean.
    *
    * Example: @c true
    **/
    boolean,
    /**
    * @brief A number.
    *
    * Example: @c 10
    **/
    number,
    /**
    * @brief A string.
    *
    * Example: @c "value"
    **/
    string,
    /**
    * @brief An array.
    *
    * Example: @c [1,2,3]
    **/
    array,
    /**
    * @brief An object.
    *
    * Example: @c {"a":"b"}
    **/
    object
};

/**
* @brief A single JSON element.
**/
struct json_element {
    using json_null = std::nullptr_t;
    using json_bool = bool;
    using json_number = long double;
    using json_string = std::string;
    using json_array = std::vector<json_element>;
    using json_object = std::map<json_string, json_element>;
    using json_element_variant = std::variant<
        json_null,
        json_bool,
        json_number,
        json_string,
        json_array,
        json_object
    >;

    json_element_variant value;

    explicit json_element() noexcept {
        this->value = nullptr;
    }
    explicit json_element(json_element_variant value) noexcept {
        this->value = std::move(value);
    }
    template<typename T> requires std::constructible_from<json_element_variant, T>
    json_element(T&& value) {
        this->value = std::forward<T>(value);
    }

    json_element_type type() const noexcept {
        return static_cast<json_element_type>(value.index());
    }

    bool is_null() const noexcept {
        return std::holds_alternative<json_null>(value);
    }
    bool is_bool() const noexcept {
        return std::holds_alternative<json_bool>(value);
    }
    bool is_number() const noexcept {
        return std::holds_alternative<json_number>(value);
    }
    bool is_string() const noexcept {
        return std::holds_alternative<json_string>(value);
    }
    bool is_array() const noexcept {
        return std::holds_alternative<json_array>(value);
    }
    bool is_object() const noexcept {
        return std::holds_alternative<json_object>(value);
    }

    json_null as_null() const noexcept {
        return std::get<json_null>(value);
    }
    json_bool as_bool() const noexcept {
        return std::get<json_bool>(value);
    }
    json_number as_number() const noexcept {
        return std::get<json_number>(value);
    }
    json_string& as_string() noexcept {
        return std::get<json_string>(value);
    }
    const json_string& as_string() const noexcept {
        return std::get<json_string>(value);
    }
    json_array& as_array() noexcept {
        return std::get<json_array>(value);
    }
    const json_array& as_array() const noexcept {
        return std::get<json_array>(value);
    }
    json_object& as_object() noexcept {
        return std::get<json_object>(value);
    }
    const json_object& as_object() const noexcept {
        return std::get<json_object>(value);
    }

    template<typename T>
    T as() const {
        // Cast to json_element
        if constexpr (std::is_same_v<T, json_element>) {
            return *this;
        }
        // Cast to json_null
        else if constexpr (std::is_same_v<T, json_null>) {
            return as_null();
        }
        // Cast to json_bool
        else if constexpr (std::is_same_v<T, json_bool>) {
            return as_bool();
        }
        // Cast to json_number
        else if constexpr (std::is_same_v<T, json_number>) {
            return as_number();
        }
        // Cast to json_string
        else if constexpr (std::is_same_v<T, json_string>) {
            return as_string();
        }
        // Cast to json_array
        else if constexpr (std::is_same_v<T, json_array>) {
            return as_array();
        }
        // Cast to json_object
        else if constexpr (std::is_same_v<T, json_object>) {
            return as_object();
        }
        // Cast to numeric type
        else if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<T>(as_number());
        }
        // Cast to map-like type
        else if constexpr (requires { typename T::key_type; typename T::mapped_type; }) {
            T result = {};
            for (const auto& [key, value] : as_object()) {
                result.emplace(key, value.as<typename T::mapped_type>());
            }
            return result;
        }
        // Cast to vector-like type
        else if constexpr (requires { typename T::value_type; }) {
            T result = {};
            for (const auto& element : as_array()) {
                result.push_back(element.as<typename T::value_type>());
            }
            return result;
        }
        // Unsupported cast
        else {
            static_assert(std::is_same_v<T, void>, "Unsupported type for json_element::as<T>()");
        }
    }
};

};

#endif // JSONH_CPP_JSON_ELEMENT_H