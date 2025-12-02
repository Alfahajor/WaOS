add_library(core_test_utils INTERFACE)
target_include_directories(core_test_utils INTERFACE ${CMAKE_SOURCE_DIR}/tests/core)

# Test Parser
add_executable(test_parser test_Parser.cpp)
target_link_libraries(test_parser PRIVATE core)
add_test(NAME ParserValidation COMMAND test_parser)

# Test Process Memory Logic
add_executable(test_process_memory test_ProcessMemory.cpp)
target_link_libraries(test_process_memory PRIVATE core)
add_test(NAME ProcessMemoryLogic COMMAND test_process_memory)

# Test Simulator Integration
add_executable(test_simulator_integration test_SimulatorIntegration.cpp)
target_link_libraries(test_simulator_integration PRIVATE core Qt6::Core core_test_utils)
add_test(NAME SimulatorIntegration COMMAND test_simulator_integration)

# Test Simulator advanced
add_executable(test_simulator_advanced test_SimulatorAdvanced.cpp)
target_link_libraries(test_simulator_advanced PRIVATE core Qt6::Core)
add_test(NAME SimulatorAdvanced COMMAND test_simulator_advanced)

# Test trace generator
add_executable(test_trace_generator test_TraceGenerator.cpp)
target_link_libraries(test_trace_generator PRIVATE core Qt6::Core)
add_test(NAME TraceGenerator COMMAND test_trace_generator)


