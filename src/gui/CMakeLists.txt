cmake_minimum_required(VERSION 3.16)

project(waos_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml)

# GUI Library sources
set(GUI_SOURCES
    controllers/SimulationController.cpp
    viewmodels/ProcessMonitorViewModel.cpp
    viewmodels/MemoryMonitorViewModel.cpp
    # viewmodels/BlockingEventsViewModel.cpp
    viewmodels/ExecutionLogViewModel.cpp
    models/ProcessItemModel.cpp
    models/FrameItemModel.cpp
    # models/EventItemModel.cpp
    mock/MockDataProvider.cpp
    mock/MockScheduler.cpp
    mock/MockMemoryManager.cpp
    mock/MockSimulator.cpp
)

set(GUI_HEADERS
    controllers/SimulationController.h
    viewmodels/ProcessMonitorViewModel.h
    viewmodels/MemoryMonitorViewModel.h
    # viewmodels/BlockingEventsViewModel.h
    viewmodels/ExecutionLogViewModel.h
    models/ProcessItemModel.h
    models/FrameItemModel.h
    # models/EventItemModel.h
    mock/MockDataProvider.h
    mock/MockScheduler.h
    mock/MockMemoryManager.h
    mock/MockSimulator.h
)

# QML Resources
set(QML_RESOURCES
    qml/resources.qrc
)

# Create GUI library
add_library(waos_gui STATIC
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${QML_RESOURCES}
)

target_include_directories(waos_gui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(waos_gui PUBLIC
    core
    scheduler
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
)

# Enable automatic MOC for Qt
set_target_properties(waos_gui PROPERTIES
    AUTOMOC ON
    AUTORCC ON
)
