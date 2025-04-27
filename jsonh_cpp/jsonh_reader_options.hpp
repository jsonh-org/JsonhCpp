#pragma once

namespace jsonh_cpp {

/// <summary>
/// Options for a jsonh_reader.
/// </summary>
struct jsonh_reader_options {
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