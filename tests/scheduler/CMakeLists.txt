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
