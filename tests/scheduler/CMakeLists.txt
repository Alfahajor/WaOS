# FCFS + Simulator Integration Test
add_executable(test_fcfs_simulator test_fcfs_simulator.cpp)

target_link_libraries(test_fcfs_simulator PRIVATE
  scheduler
  core
  Qt6::Core
)

add_test(NAME FCFSSimulatorIntegration COMMAND test_fcfs_simulator)
