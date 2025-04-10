# JsonhCpp

**JSON for Humans.**

JSON is great. Until you miss that trailing comma... or want to use comments. What about multiline strings?
JSONH provides a much more elegant way to write JSON that's designed for humans rather than machines.

Since JSONH is compatible with JSON, any JSONH syntax can be represented with equivalent JSON.

JsonhCpp is an implementation of [JSONH v1](https://github.com/jsonh-org/Jsonh).

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
#include "jsonh_cpp.hpp" // for jsonh

std::string jsonh = R"(
{
    this is: awesome
}
)";
std::string element = jsonh::jsonh_reader::parse_element<std::string>(jsonh).value();
```

## Dependencies

_Updated: 2025/04/11_

- [nlohmann/json](https://github.com/nlohmann/json)
- [catchorg/Catch2](https://github.com/catchorg/Catch2) ([help](https://stackoverflow.com/a/78804393))
- C++ 23
- C 23