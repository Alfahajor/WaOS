#include "waos/core/Parser.h"
#include "waos/core/Process.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdio>

using namespace waos::core;

// Helper Function: Create a temporary file for testing
std::string createTestFile(const std::string& filename, const std::string& content) {
  std::ofstream out(filename);
  out << content;
  out.close();
  return filename;
}

// Helper Function: Delete the temporary file
void removeTestFile(const std::string& filename) {
  std::remove(filename.c_str());
}

// Test Case 1: Standard Format with Mixed Bursts
void test_standard_mixed_bursts() {
  std::cout << "[RUNNING] test_standard_mixed_bursts..." << std::endl;

  std::string content = "P1 0 CPU(4),E/S(3),CPU(5) 1 10\n";
  std::string filename = "test_1.txt";
  createTestFile(filename, content);

  try {
    auto processes = Parser::parseFile(filename);

    // Check basic counts
    assert(processes.size() == 1);
    const auto& p1 = processes[0];

    // Check static data
    assert(p1.pid == 1);
    assert(p1.arrivalTime == 0);
    assert(p1.priority == 1);
    assert(p1.requiredPages == 10);

    // Check Bursts Queue
    // Copy queue to inspect it non-destructively
    std::queue<Burst> q = p1.bursts;
    
    assert(q.size() == 3);

    // Burst 1: CPU(4)
    assert(q.front().type == BurstType::CPU);
    assert(q.front().duration == 4);
    q.pop();

    // Burst 2: E/S(3)
    assert(q.front().type == BurstType::IO);
    assert(q.front().duration == 3);
    q.pop();

    // Burst 3: CPU(5)
    assert(q.front().type == BurstType::CPU);
    assert(q.front().duration == 5);
    q.pop();

    std::cout << "[PASSED] test_standard_mixed_bursts" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "[FAILED] " << e.what() << std::endl;
    assert(false);
  }
  
  removeTestFile(filename);
}

// Test Case 2: Multiple Processes & Comments
void test_multiple_processes_and_comments() {
  std::cout << "[RUNNING] test_multiple_processes_and_comments..." << std::endl;

  std::string content = 
    "# This is a comment\n"
    "P10 2 CPU(10) 5 4\n"
    "\n"
    "P20 4 E/S(5),CPU(2) 1 8\n";
  
  std::string filename = "test_2.txt";
  createTestFile(filename, content);

  auto processes = Parser::parseFile(filename);

  assert(processes.size() == 2);

  // Check P10
  assert(processes[0].pid == 10);
  assert(processes[0].bursts.front().type == BurstType::CPU);
  assert(processes[0].bursts.front().duration == 10);

  // Check P20
  assert(processes[1].pid == 20);
  assert(processes[1].bursts.front().type == BurstType::IO);
  assert(processes[1].bursts.front().duration == 5);

  std::cout << "[PASSED] test_multiple_processes_and_comments" << std::endl;
  removeTestFile(filename);
}

// Test Case 3: Invalid Lines (Should skip, not crash)
void test_robustness_invalid_lines() {
  std::cout << "[RUNNING] test_robustness_invalid_lines..." << std::endl;

  std::string content = 
    "P1 0 INVALID(4) 1 1\n"
    "BROKEN_LINE_HERE\n"
    "P2 5 CPU(4) 1 2\n";
  
  std::string filename = "test_3.txt";
  createTestFile(filename, content);

  // Should only load the valid P2
  auto processes = Parser::parseFile(filename);

  assert(processes.size() == 1);
  assert(processes[0].pid == 2);

  std::cout << "[PASSED] test_robustness_invalid_lines" << std::endl;
  removeTestFile(filename);
}

int main() {
  std::cout << "> Starting Parser Tests" << std::endl;
  
  test_standard_mixed_bursts();
  std::cout << std::endl;
  test_multiple_processes_and_comments();
  std::cout << std::endl;
  test_robustness_invalid_lines();

  std::cout << "< All Parser Tests Passed" << std::endl;
  return 0;
}
