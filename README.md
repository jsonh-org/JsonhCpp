<img src="https://github.com/jsonh-org/Jsonh/blob/main/IconUpscaled.png?raw=true" width=180>

[![C++](https://img.shields.io/github/release/jsonh-org/JsonhCpp.svg?label=c%2b%2b)](https://github.com/jsonh-org/JsonhCpp/releases)

**JSON for Humans.**

JSON is great. Until you miss that trailing comma... or want to use comments. What about multiline strings?
JSONH provides a much more elegant way to write JSON that's designed for humans rather than machines.

Since JSONH is compatible with JSON, any JSONH syntax can be represented with equivalent JSON.

## JsonhCpp

JsonhCpp is a parser implementation of [JSONH V1 & V2](https://github.com/jsonh-org/Jsonh) for C++.

## Example

```jsonh
{
    // use #, // or /**/ comments
    
    // quotes are optional
    keys: without quotes,

    // commas are optional
    isn\'t: {
        that: cool? # yes
    }

    // use multiline strings
    haiku: '''
        Let me die in spring
          beneath the cherry blossoms
            while the moon is full.
        '''
    
    // compatible with JSON5
    key: 0xDEADCAFE

    // or use JSON
    "old school": 1337
}
```

## Usage

Everything you need is contained within `jsonh_reader`:

```cpp
#include "jsonh_cpp.hpp"

std::string jsonh = R"(
{
    this is: awesome
}
)";
json_element::json_object element = jsonh_cpp::jsonh_reader::parse_element<json_element::json_object>(jsonh).value();
```

## Dependencies

- C++23
- [catchorg/Catch2](https://github.com/catchorg/Catch2) (v3.16.1)
- [rindeal/Amalgamate](https://github.com/rindeal/Amalgamate) (v0.99.0)

## Limitations

In comparison to [JsonhCs](https://github.com/jsonh-org/JsonhCs), this C++ implementation has some limitations.

### UTF-8 only

The input stream must be in UTF-8 encoding.

If using a different encoding, consider converting to UTF-8 using [utfcpp](https://github.com/nemtrif/utfcpp).

### Fixed-size numbers

There is no option to parse numbers with arbitrary range/precision.
Numbers are parsed as `long double`, which is generally 64-bit.