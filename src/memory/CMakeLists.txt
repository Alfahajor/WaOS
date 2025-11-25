add_library(memory STATIC
    FIFOMemoryManager.cpp
    LRUMemoryManager.cpp
    OptimalMemoryManager.cpp
)

target_include_directories(memory PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(memory PUBLIC core)
