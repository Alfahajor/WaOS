# Enable CTest functionality
enable_testing()

# Test parser
add_executable(test_parser core/test_Parser.cpp)
target_link_libraries(test_parser PRIVATE core)
add_test(NAME ParserValidation COMMAND test_parser)

# Test Process Memory Logic (NEW)
add_executable(test_process_memory core/test_ProcessMemory.cpp)
target_link_libraries(test_process_memory PRIVATE core)
add_test(NAME ProcessMemoryLogic COMMAND test_process_memory)

# Test Simulator Integration (NEW - IO/PageFault)
add_executable(test_simulator_integration core/test_SimulatorIntegration.cpp)
target_link_libraries(test_simulator_integration PRIVATE core Qt6::Core)
add_test(NAME SimulatorIntegration COMMAND test_simulator_integration)
