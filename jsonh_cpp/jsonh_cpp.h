#include <iostream>
#include <sstream>
#include <string>
#include <locale>
#include "tinyutf8/tinyutf8.h"

class jsonh_reader_options final {
public:
	bool incomplete_inputs = false;
};

class jsonh_reader final {
public:
	/// <summary>
	/// The stream to read characters from.
	/// </summary>
	std::istream* stream;
	/// <summary>
	/// The options to use when reading JSONH.
	/// </summary>
	jsonh_reader_options* options;
	/// <summary>
	/// The number of characters to read from <see cref="stream"/>.
	/// </summary>
	long char_counter;

	/// <summary>
	/// Constructs a reader that reads JSONH from a stream.
	/// </summary>
	jsonh_reader(std::istream* stream, jsonh_reader_options* options = nullptr) {
		this->stream = stream;
		if (options == nullptr) {
			options = new jsonh_reader_options();
		}
		this->options = options;
		//Stream->imbue(std::locale(new std::codecvt_utf8<char>()))
	}
	/// <summary>
	/// Constructs a reader that reads JSONH from a string.
	/// </summary>
	jsonh_reader(std::string* string, jsonh_reader_options* options = nullptr) : jsonh_reader(new std::istringstream(*string), options) {
	}

private:
	const char32_t RESERVED_CHARS[11] = { U'\\', U',', U':', U'[', U']', U'{', U'}', U'/', U'#', U'"', U'\'' };
	const char32_t NEWLINE_CHARS[4] = { U'\n', U'\r', U'\u2028', U'\u2029' };
};