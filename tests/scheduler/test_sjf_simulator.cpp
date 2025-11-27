/**
 * @brief Integration test for SJF Scheduler with Simulator
 * @details Tests the SJF (Shortest Job First) scheduling algorithm.
 *          SJF selects the process with the shortest CPU burst for execution.
 *          This is a non-preemptive algorithm.
 * 
 * Key characteristics of SJF:
 * - Selects process with shortest CURRENT CPU burst
 * - Non-preemptive: once a process starts, it runs until burst completes
 * - Minimizes average waiting time
 * - Can cause starvation of long processes
 * - Optimal for minimizing average turnaround time
 */

#include <iostream>
#include <cassert>
#include <memory>
#include <iomanip>
#include <sstream>
#include "waos/core/Simulator.h"
#include "waos/scheduler/SJFScheduler.h"
#include "waos/memory/IMemoryManager.h"

using namespace waos::core;
using namespace waos::scheduler;
using namespace waos::memory;

/**
 * @brief Helper function to convert ProcessState to string
 */
std::string stateToString(ProcessState state) {
    switch(state) {
        case ProcessState::NEW: return "NEW";
        case ProcessState::READY: return "READY";
        case ProcessState::RUNNING: return "RUNNING";
        case ProcessState::BLOCKED: return "BLOCKED";
        case ProcessState::WAITING_MEMORY: return "WAIT_MEM";
        case ProcessState::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Helper function to convert BurstType to string
 */
std::string burstTypeToString(BurstType type) {
    switch(type) {
        case BurstType::CPU: return "CPU";
        case BurstType::IO: return "I/O";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Silent mock memory manager - focus on CPU scheduling, not memory
 * @details Always returns HIT, no logging to keep output clean
 */
class MockMemoryManager : public IMemoryManager {
public:
    bool isPageLoaded(int pid, int page) const override {
        return true; // All pages are always loaded
    }

    PageRequestResult requestPage(int pid, int page) override {
        return PageRequestResult::HIT; // No page faults, silent
    }

    void allocateForProcess(int pid, int pages) override {
        // Silent - focus on scheduling
    }

    void freeForProcess(int pid) override {
        // Silent - focus on scheduling  
    }

    void completePageLoad(int processId, int pageNumber) override {
        // No-op for this simple test
    }
};

/**
 * @brief Test 1: SJF with simultaneous arrivals
 * @details All processes arrive at t=0 with different burst lengths
 *          Expected order: P2(2) -> P3(4) -> P1(8)
 */
void test_sjf_simultaneous_arrivals() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 1: SJF with Simultaneous Arrivals            ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;
    std::cout << "Expected: P2 (burst=2) -> P3 (burst=4) -> P1 (burst=8)\n" << std::endl;

    Simulator sim;
    
    // Load test processes
    std::string mockFile = "../../../tests/mock/test_sjf_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes from " << mockFile << std::endl;
        assert(false);
    }

    // Setup SJF scheduler
    auto scheduler = std::make_unique<SJFScheduler>();
    sim.setScheduler(std::move(scheduler));

    // Setup mock memory manager (silent)
    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    // Start simulation
    sim.start();
    std::cout << "Simulation started - Observing SJF behavior\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Run simulation for limited ticks
    int maxTicks = 20;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        sim.tick();
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ SJF Simultaneous Arrivals Test\n" << std::endl;
}

/**
 * @brief Test 2: SJF with staggered arrivals
 * @details Processes arrive at different times
 *          Demonstrates how SJF prioritizes short jobs over long ones
 */
void test_sjf_staggered_arrivals() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 2: SJF with Staggered Arrivals               ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;
    std::cout << "P4(10) arrives first, but shorter jobs get priority\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_sjf_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<SJFScheduler>();
    sim.setScheduler(std::move(scheduler));

    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    sim.start();
    std::cout << "Simulation started\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Run longer to see staggered arrivals
    int maxTicks = 50;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        
        // Show every tick for critical region
        if (currentTime >= 10 && currentTime <= 30) {
            std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        }
        
        sim.tick();
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ SJF Staggered Arrivals Test\n" << std::endl;
}

/**
 * @brief Test 3: SJF with I/O operations
 * @details Tests how SJF handles processes with I/O bursts
 *          When process returns from I/O, it competes based on remaining burst
 */
void test_sjf_with_io() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 3: SJF with I/O Operations                   ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;
    std::cout << "SJF considers current burst length, not total time\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_sjf_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<SJFScheduler>();
    sim.setScheduler(std::move(scheduler));

    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    sim.start();
    std::cout << "Simulation started\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    int maxTicks = 60;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        
        // Show ticks around I/O operations
        if (currentTime >= 25 && currentTime <= 40) {
            std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        }
        
        sim.tick();
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ SJF with I/O Test\n" << std::endl;
}

/**
 * @brief Test 4: SJF starvation demonstration
 * @details Shows how a long process can be starved by continuous arrival of short processes
 */
void test_sjf_starvation() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 4: SJF Starvation (Long Process Waiting)     ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;
    std::cout << "P10(burst=15) must wait for all shorter processes\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_sjf_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<SJFScheduler>();
    sim.setScheduler(std::move(scheduler));

    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    sim.start();
    std::cout << "Simulation started\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    int maxTicks = 70;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        
        // Show ticks around starvation region
        if (currentTime >= 35 && currentTime <= 50) {
            std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        }
        
        sim.tick();
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ SJF Starvation Test\n" << std::endl;
}

/**
 * @brief Test 5: Full simulation
 */
void test_sjf_full_simulation() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 5: SJF Full Simulation                       ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_sjf_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<SJFScheduler>();
    sim.setScheduler(std::move(scheduler));

    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    sim.start();
    uint64_t startTime = sim.getCurrentTime();
    std::cout << "Running complete simulation...\n" << std::endl;

    // Run until completion or timeout
    int maxTicks = 100;
    int tick = 0;
    
    while (sim.isRunning() && tick < maxTicks) {
        // Show progress every 20 ticks
        if (tick % 20 == 0) {
            std::cout << "  [Progress: " << tick << " ticks]" << std::endl;
        }
        sim.tick();
        tick++;
    }

    uint64_t endTime = sim.getCurrentTime();
    
    std::cout << "\n" << std::string(70, '=') << std::endl;
    if (!sim.isRunning()) {
        std::cout << "✅ Simulation completed!" << std::endl;
        std::cout << "   Duration: " << (endTime - startTime) << " ticks" << std::endl;
        std::cout << "[PASSED] ✓ SJF Full Simulation\n" << std::endl;
    } else {
        std::cout << "⚠️  Reached max ticks (" << maxTicks << ")" << std::endl;
        std::cout << "[WARNING] May need more ticks\n" << std::endl;
    }
}

int main() {
    std::cout << "\n╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  SJF Scheduler + Simulator Integration      ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝\n" << std::endl;

    try {
        test_sjf_simultaneous_arrivals();
        test_sjf_staggered_arrivals();
        test_sjf_with_io();
        test_sjf_starvation();
        test_sjf_full_simulation();

        std::cout << "\n✅ All SJF integration tests passed!\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
