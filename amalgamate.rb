AMALGAMATE_PATH = 'amalgamate-v0.99.0-windows-amd64/amalgamate.exe' # https://github.com/rindeal/Amalgamate
INPUT_PATH = "jsonh_cpp/jsonh_cpp.hpp"
OUTPUT_PATH = "single_include/jsonh_cpp_amalgamated.hpp"

HEADER = "// JsonhCpp (JSON for Humans)
// Version: 5.1
// Link: https://github.com/jsonh-org/JsonhCpp
// License: MIT"

# Amalgamate source to single file
system("#{AMALGAMATE_PATH} \"#{INPUT_PATH}\" \"#{OUTPUT_PATH}\"")

# Prepend header
File.write(OUTPUT_PATH, HEADER + "\n\n" + File.read(OUTPUT_PATH))

puts("All done!")
gets()