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
#include "jsonh_cpp.hpp" // for jsonh_cpp

std::string jsonh = R"(
{
    this is: awesome
}
)";
std::string element = jsonh_cpp::jsonh_reader::parse_element<std::string>(jsonh).value();
```

## Dependencies

- C++20
- [nlohmann/json](https://github.com/nlohmann/json) (v3.12.0 - patch 2025/11/19 18:26)
- [catchorg/Catch2](https://github.com/catchorg/Catch2) (v3.11.0) - [help](https://stackoverflow.com/a/78804393)
- [martinmoene/expected-lite](https://github.com/martinmoene/expected-lite) (v0.10.0) - backport
- [rindeal/Amalgamate](https://github.com/rindeal/Amalgamate) (v0.99.0)

## Limitations

In comparison to [JsonhCs](https://github.com/jsonh-org/JsonhCs), this C++ implementation has some limitations.

### UTF-8 only

The input stream must be in UTF-8 encoding.

If using a different encoding, consider converting to UTF-8 using [utfcpp](https://github.com/nemtrif/utfcpp).

### Fixed-size numbers

Numbers are parsed as `long long` and `long double`.
In general, these are 64-bit and have a range of about 9 quintillion and a precision of about 15 decimal places.

### No token streaming

While tokens can be read one by one from a stream, the tokens are aggregated in a `std::vector`
before returning due to a lack of `yield` in C++.