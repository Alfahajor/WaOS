/**
 * @brief Unit tests for Scheduling Algorithms.
 * Validates SJF ordering and Priority ordering using the updated Process* API.
 */

#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include "waos/scheduler/FCFSScheduler.h"
#include "waos/scheduler/SJFScheduler.h"
#include "waos/scheduler/PriorityScheduler.h"
#include "waos/core/Process.h"

using namespace waos::core;
using namespace waos::scheduler;

// Helper para crear procesos fácilmente
std::unique_ptr<Process> createProcess(int pid, int burstDuration, int priority) {
    std::queue<Burst> bursts;
    bursts.push({BurstType::CPU, burstDuration});
    // Arrival 0, Pages 1
    return std::make_unique<Process>(pid, 0, priority, bursts, 1);
}

void test_sjf_ordering() {
    std::cout << "[RUNNING] test_sjf_ordering..." << std::endl;

    SJFScheduler sjf;
    
    // El simulador posee los procesos
    std::vector<std::unique_ptr<Process>> processes;
    processes.push_back(createProcess(1, 10, 0)); // Long
    processes.push_back(createProcess(2, 2, 0));  // Short
    processes.push_back(createProcess(3, 5, 0));  // Medium

    for(const auto& p : processes) {
        sjf.addProcess(p.get());
    }

    // Esperamos orden: P2 (2), P3 (5), P1 (10)
    Process* p;
    
    p = sjf.getNextProcess();
    assert(p != nullptr);
    assert(p->getPid() == 2);

    p = sjf.getNextProcess();
    assert(p != nullptr);
    assert(p->getPid() == 3);

    p = sjf.getNextProcess();
    assert(p != nullptr);
    assert(p->getPid() == 1);

    assert(sjf.getNextProcess() == nullptr);
    std::cout << "[PASSED] test_sjf_ordering" << std::endl;
}

void test_priority_ordering() {
    std::cout << "[RUNNING] test_priority_ordering..." << std::endl;

    PriorityScheduler prio;

    std::vector<std::unique_ptr<Process>> processes;
    processes.push_back(createProcess(1, 10, 10)); // Low Priority (High value)
    processes.push_back(createProcess(2, 10, 1));  // High Priority (Low value)
    processes.push_back(createProcess(3, 10, 5));  // Medium Priority

    for(const auto& p : processes) {
        prio.addProcess(p.get());
    }

    // Esperamos orden: P2 (Prio 1), P3 (Prio 5), P1 (Prio 10)
    Process* p;

    p = prio.getNextProcess();
    assert(p != nullptr);
    assert(p->getPid() == 2);

    p = prio.getNextProcess();
    assert(p != nullptr);
    assert(p->getPid() == 3);

    p = prio.getNextProcess();
    assert(p != nullptr);
    assert(p->getPid() == 1);

    std::cout << "[PASSED] test_priority_ordering" << std::endl;
}

int main() {
    std::cout << "> Scheduler Logic Validation\n";
    
    test_sjf_ordering();
    test_priority_ordering();

    std::cout << "< All Scheduler Tests Passed\n";
    return 0;
}
