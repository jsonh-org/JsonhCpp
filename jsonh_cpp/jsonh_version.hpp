#pragma once

namespace jsonh_cpp {

/**
* @brief The major versions of the JSONH specification.
**/
enum struct jsonh_version {
    /**
    * @brief Indicates that the latest version should be used (currently @ref v2).
    **/
    latest = 0,
    /**
    * @brief Version 1 of the specification, released 2025/03/19.
    **/
    v1 = 1,
    /**
    * @brief Version 2 of the specification, released 2025/11/19.
    **/
    v2 = 2,
};

}