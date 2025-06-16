#pragma once

namespace jsonh_cpp {

/// <summary>
/// The types of tokens that make up a JSON document.
/// </summary>
enum struct json_token_type : char {
    /// <summary>
    /// Indicates that there is no value (not to be confused with <see cref="null"/>).
    /// </summary>
    none = 0,
    /// <summary>
    /// The start of an object.<br/>
    /// Example: <c>{</c>
    /// </summary>
    start_object = 1,
    /// <summary>
    /// The end of an object.<br/>
    /// Example: <c>}</c>
    /// </summary>
    end_object = 2,
    /// <summary>
    /// The start of an array.<br/>
    /// Example: <c>&#x005B;</c>
    /// </summary>
    start_array = 3,
    /// <summary>
    /// The end of an array.<br/>
    /// Example: <c>&#x005D;</c>
    /// </summary>
    end_array = 4,
    /// <summary>
    /// A property name in an object.<br/>
    /// Example: <c>"key":</c>
    /// </summary>
    property_name = 5,
    /// <summary>
    /// A comment.<br/>
    /// Example: <c>// comment</c>
    /// </summary>
    comment = 6,
    /// <summary>
    /// A string.<br/>
    /// Example: <c>"value"</c>
    /// </summary>
    string = 7,
    /// <summary>
    /// A number.<br/>
    /// Example: <c>10</c>
    /// </summary>
    number = 8,
    /// <summary>
    /// A true boolean.<br/>
    /// Example: <c>true</c>
    /// </summary>
    true_bool = 9,
    /// <summary>
    /// A false boolean.<br/>
    /// Example: <c>false</c>
    /// </summary>
    false_bool = 10,
    /// <summary>
    /// A null value.<br/>
    /// Example: <c>null</c>
    /// </summary>
    null = 11,
};

}