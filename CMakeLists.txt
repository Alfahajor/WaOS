cmake_minimum_required(VERSION 3.16)

project(OperatingSystemSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6 configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Agregamos el subdirectorio que contiene la lógica del núcleo.
# Este subdirectorio tendrá su propio CMakeLists.txt.
add_subdirectory(src/core)
# add_subdirectory(src/scheduler)
# add_subdirectory(src/memory)

# target_link_libraries(simulator PRIVATE
#   core
#   Qt6::Core
#   Qt6::Gui
#   Qt6::Widgets
# )

# Simulation main executable
# add_executable(simulator
#   src/ui/MainWindow.cpp
#   src/ui/MainWindow.h
#   src/ui/MainWindow.ui
#   src/main.cpp
# )

# Enable CTest
enable_testing()

# Add test subdirectory
add_subdirectory(tests)
