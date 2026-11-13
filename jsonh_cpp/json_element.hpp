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