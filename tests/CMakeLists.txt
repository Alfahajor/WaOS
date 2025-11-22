# Enable CTest functionality
enable_testing()

# Define the test executable
add_executable(test_parser core/test_Parser.cpp)

# Link against the core library (where Parser and Process are defined)
target_link_libraries(test_parser PRIVATE core)

# Register the test with CTest so we can run it using 'ctest' command
add_test(NAME ParserValidation COMMAND test_parser)
