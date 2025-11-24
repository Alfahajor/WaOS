add_executable(test_memory test_memory_main.cpp)

target_link_libraries(test_memory PRIVATE
  core
)

add_test(NAME MemoryStructures COMMAND test_memory)
