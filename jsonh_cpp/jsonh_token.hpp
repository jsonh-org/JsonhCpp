#pragma once

#include <string> // for std::string
#include "jsonh_reader.hpp" // for jsonh::jsonh_reader
#include "jsonh_token_type.hpp" // for jsonh::jsonh_token_type

namespace jsonh {
/// <summary>
/// A single JSONH token with a <see cref="json_token_type"/>.
/// </summary>
struct jsonh_token {
    /// <summary>
    /// The <see cref="JsonhReader"/> that read the token.
    /// </summary>
    jsonh_reader* reader;
    /// <summary>
    /// The type of the token.
    /// </summary>
    json_token_type json_type;
    /// <summary>
    /// The value of the token, or an empty string.
    /// </summary>
    std::string value = value;

    /// <summary>
    /// Constructs a single JSONH token.
    /// </summary>
    jsonh_token(jsonh_reader* reader, json_token_type json_type, std::string value = "") {
        this->reader = reader;
        this->json_type = json_type;
        this->value = value;
    }
};
}