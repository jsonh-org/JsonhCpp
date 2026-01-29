#pragma once

#include "jsonh_version.hpp" // for jsonh_cpp::jsonh_version

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