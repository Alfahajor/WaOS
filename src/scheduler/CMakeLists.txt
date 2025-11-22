add_library(scheduler STATIC
  FCFSScheduler.cpp
  SJFScheduler.cpp
  RRScheduler.cpp
  PriorityScheduler.cpp
)

target_include_directories(scheduler PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(scheduler PUBLIC
  core
)

add_executable(test_scheduler
  test_scheduler_main.cpp
)

target_link_libraries(test_scheduler PRIVATE
  scheduler
  core
)

