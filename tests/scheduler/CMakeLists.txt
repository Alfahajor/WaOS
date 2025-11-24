add_executable(test_scheduler test_scheduler_main.cpp)

target_link_libraries(test_scheduler PRIVATE
  scheduler
  core
)

add_test(NAME SchedulerLogic COMMAND test_scheduler)
