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
};

}