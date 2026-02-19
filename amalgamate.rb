HEADER = "// JsonhCpp (JSON for Humans)
// Version: 6.5
// Link: https://github.com/jsonh-org/JsonhCpp
// License: MIT"

AMALGAMATE_PATH = "amalgamate-v0.99.0-windows-amd64/amalgamate.exe"
INPUT_PATH = "jsonh_cpp/jsonh_cpp.hpp"
OUTPUT_PATH = "single_include/jsonh_cpp_amalgamated.hpp"

# Amalgamate source to single file
system("#{AMALGAMATE_PATH} \"#{INPUT_PATH}\" \"#{OUTPUT_PATH}\"")

# Prepend header
File.write(OUTPUT_PATH, HEADER + "\n\n" + File.read(OUTPUT_PATH))

puts("All done!")
gets()