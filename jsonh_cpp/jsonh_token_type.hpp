#pragma once

namespace jsonh_cpp {

/**
* @brief The types of tokens that make up a JSON document.
**/
enum struct json_token_type : char {
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