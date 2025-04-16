#pragma once

namespace jsonh {

/// <summary>
/// Options for a jsonh_reader.
/// </summary>
struct jsonh_reader_options final {
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