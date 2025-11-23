# Qt6 Core is required because Simulator inherits from QObject
find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(core STATIC
  Process.cpp 
  Clock.cpp 
  Parser.cpp
  Simulator.cpp
  ${PROJECT_SOURCE_DIR}/include/waos/core/Simulator.h
)

# Required if any header/class uses Q_OBJECT
set_target_properties(core PROPERTIES
  AUTOMOC ON
  AUTOUIC ON
  AUTORCC ON
)

target_include_directories(core PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(core
  PUBLIC Qt6::Core
)
