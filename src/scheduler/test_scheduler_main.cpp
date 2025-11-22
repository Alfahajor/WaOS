/**
 * @file test_scheduler_main.cpp
 * @brief Small integration test to verify scheduler stubs compile and run.
 *
 * This program creates simple processes and exercises each scheduler stub.
 * Adjust Process construction to match your Process API if needed.
 */

#include <iostream>
#include "waos/scheduler/FCFSScheduler.h"
#include "waos/scheduler/SJFScheduler.h"
#include "waos/scheduler/RRScheduler.h"
#include "waos/scheduler/PriorityScheduler.h"
#include "waos/core/Process.h"

int main() {
    std::cout << "=== Scheduler stubs integration test ===\n";

    // Create CPU burst queues for processes using Burst structs
    std::queue<waos::core::Burst> bursts1;
    bursts1.push({waos::core::BurstType::CPU, 5});
    bursts1.push({waos::core::BurstType::CPU, 3});

    std::queue<waos::core::Burst> bursts2;
    bursts2.push({waos::core::BurstType::CPU, 2});

    std::queue<waos::core::Burst> bursts3;
    bursts3.push({waos::core::BurstType::CPU, 4});
    bursts3.push({waos::core::BurstType::CPU, 1});

    // Instantiating waos::core::Process per provided API:
    waos::core::Process* p1 = new waos::core::Process(1, /*arrivalTime*/ 0, bursts1, /*requiredPages*/ 4);
    waos::core::Process* p2 = new waos::core::Process(2, /*arrivalTime*/ 1, bursts2, /*requiredPages*/ 2);
    waos::core::Process* p3 = new waos::core::Process(3, /*arrivalTime*/ 2, bursts3, /*requiredPages*/ 1);

    // FCFS test
    {
        waos::scheduler::FCFSScheduler s;
        s.addProcess(p1);
        s.addProcess(p2);
        while (s.hasReadyProcesses()) {
            auto* p = s.getNextProcess();
            (void)p;
        }
    }

    // SJF test
    {
        waos::scheduler::SJFScheduler s;
        s.addProcess(p3);
        auto* p = s.getNextProcess();
        (void)p;
    }

    // RR test
    {
        waos::scheduler::RRScheduler s(std::chrono::milliseconds(50));
        s.addProcess(p1);
        s.addProcess(p2);
        while (s.hasReadyProcesses()) {
            auto* p = s.getNextProcess();
            (void)p;
        }
    }

    // Priority test
    {
        waos::scheduler::PriorityScheduler s;
        s.addProcess(p2);
        s.addProcess(p3);
        auto* p = s.getNextProcess();
        (void)p;
    }

    delete p1;
    delete p2;
    delete p3;

    std::cout << "=== Test finished ===\n";
    return 0;
}
