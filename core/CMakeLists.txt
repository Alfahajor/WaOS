add_library(core STATIC
  src/Process.cpp 
  src/Clock.cpp 
)

target_include_directories(core PUBLIC
  ../include 
)
