#pragma once

namespace jsonh_cpp {

/// <summary>
/// The major versions of the JSONH specification.
/// </summary>
enum struct jsonh_version {
    /// <summary>
    /// Indicates that the latest version should be used (currently v1).
    /// </summary>
    latest = 0,
    /// <summary>
    /// Version 1 of the specification, released 2025/03/19.
    /// </summary>
    v1 = 1,
};

}