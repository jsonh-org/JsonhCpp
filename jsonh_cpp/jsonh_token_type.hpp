#pragma once

namespace jsonh {

enum struct json_token_type {
    none = 0,
    start_object = 1,
    end_object = 2,
    start_array = 3,
    end_array = 4,
    property_name = 5,
    comment = 6,
    string = 7,
    number = 8,
    true_bool = 9,
    false_bool = 10,
    null = 11,
};

}