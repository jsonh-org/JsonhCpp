#ifndef JSONH_CPP_JSONH_TOKEN_H
#define JSONH_CPP_JSONH_TOKEN_H

#include <string>
#include "json_token_type.hpp"

namespace jsonh_cpp {

/**
* @brief A single JSONH token with a @ref json_token_type.
**/
struct jsonh_token {
    /**
    * @brief The type of the token.
    **/
    json_token_type json_type;
    /**
    * @brief The value of the token, or an empty string.
    **/
    std::string value;

    /**
    * @brief Constructs a single JSONH token.
    **/
    jsonh_token(json_token_type json_type, std::string value = "") noexcept {
        this->json_type = json_type;
        this->value = value;
    }
};

}

#endif // JSONH_CPP_JSONH_TOKEN_H