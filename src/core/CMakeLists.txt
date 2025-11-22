add_library(core STATIC
  Process.cpp 
  Clock.cpp 
  Parser.cpp
)

target_include_directories(core PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)
