# FCFS + Simulator Integration Test
add_executable(test_fcfs_simulator test_fcfs_simulator.cpp)

target_link_libraries(test_fcfs_simulator PRIVATE
  scheduler
  core
  Qt6::Core
)

add_test(NAME FCFSSimulatorIntegration COMMAND test_fcfs_simulator)

# SJF + Simulator Integration Test
add_executable(test_sjf_simulator test_sjf_simulator.cpp)

target_link_libraries(test_sjf_simulator PRIVATE
  scheduler
  core
  Qt6::Core
)

add_test(NAME SJFSimulatorIntegration COMMAND test_sjf_simulator)

# RR + Simulator Integration Test
add_executable(test_rr_simulator test_rr_simulator.cpp)

target_link_libraries(test_rr_simulator PRIVATE
  scheduler
  core
  Qt6::Core
)

add_test(NAME RRSimulatorIntegration COMMAND test_rr_simulator)

# Priority + Simulator Integration Test
add_executable(test_priority_simulator test_priority_simulator.cpp)

target_link_libraries(test_priority_simulator PRIVATE
  scheduler
  core
  Qt6::Core
)

add_test(NAME PrioritySimulatorIntegration COMMAND test_priority_simulator)
