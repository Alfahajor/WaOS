add_executable(test_memory test_memory_main.cpp)
target_link_libraries(test_memory PRIVATE core)
add_test(NAME MemoryStructures COMMAND test_memory)

# FIFO Memory Manager Test
add_executable(test_fifo_memory test_FIFOMemoryManager.cpp)
target_link_libraries(test_fifo_memory PRIVATE memory core)
add_test(NAME FIFOMemoryManager COMMAND test_fifo_memory)

# LRU Memory Manager Test
add_executable(test_lru_memory test_LRUMemoryManager.cpp)
target_link_libraries(test_lru_memory PRIVATE memory core)
add_test(NAME LRUMemoryManager COMMAND test_lru_memory)

# Optimal Memory Manager Integration Test (with Process)
add_executable(test_optimal_memory test_OptimalMemoryManager.cpp)
target_link_libraries(test_optimal_memory PRIVATE memory core)
add_test(NAME OptimalMemoryManager COMMAND test_optimal_memory)