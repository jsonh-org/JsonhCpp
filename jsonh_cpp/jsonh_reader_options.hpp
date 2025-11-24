#pragma once

#include "jsonh_version.hpp" // for jsonh_cpp::jsonh_version

namespace jsonh_cpp {

/// <summary>
/// Options for a jsonh_reader.
/// </summary>
struct jsonh_reader_options {
    /// <summary>
    /// Specifies the major version of the JSONH specification to use.
    /// </summary>
    jsonh_version version = jsonh_version::latest;
    /// <summary>
    /// Enables/disables parsing unclosed inputs.
    /// <code>
    /// {
    ///   "key": "val
    /// </code>
    /// This is potentially useful for large language models that stream responses.<br/>
    /// Only some tokens can be incomplete in this mode, so it should not be relied upon.
    /// </summary>
    bool incomplete_inputs = false;
    /// <summary>
    /// Enables/disables checks for exactly one element when parsing.
    /// <code>
    /// "cat"
    /// "dog" // Error: Expected single element
    /// </code>
    /// This option does not apply when reading elements, only when parsing elements.
    /// </summary>
    bool parse_single_element = false;

    /// <summary>
    /// Returns whether <see cref="version"/> is greater than or equal to <paramref name="minimum_version"/>.
    /// </summary>
    bool supports_version(jsonh_version minimum_version) const noexcept {
        const jsonh_version latest_version = jsonh_version::v2;

        jsonh_version options_version = version == jsonh_version::latest ? latest_version : version;
        jsonh_version given_version = minimum_version == jsonh_version::latest ? latest_version : minimum_version;

        return options_version >= given_version;
    }
};

}