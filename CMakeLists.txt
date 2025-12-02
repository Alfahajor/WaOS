cmake_minimum_required(VERSION 3.16)

project(OperatingSystemSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6 configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick Qml QuickControls2)

# Agregamos el subdirectorio que contiene la lógica del núcleo.
# Este subdirectorio tendrá su propio CMakeLists.txt.
add_subdirectory(src/core)
add_subdirectory(src/scheduler)
add_subdirectory(src/memory)

# GUI Module (Qt Quick)
add_subdirectory(src/gui)

# Main executable for GUI application
add_executable(waos_simulator
  src/gui/main.cpp
)

target_link_libraries(waos_simulator PRIVATE
  core
  scheduler
  waos_gui
  Qt6::Core
  Qt6::Quick
  Qt6::Qml
  Qt6::QuickControls2
)

# Enable CTest
enable_testing()

# Add test subdirectory
add_subdirectory(tests)