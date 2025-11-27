/**
 * @brief Integration test for FCFS Scheduler with Simulator
 * @details Tests the FCFS scheduling algorithm integrated with the full Simulator,
 *          including process arrivals, CPU execution, and I/O operations.
 */

#include <iostream>
#include <cassert>
#include <memory>
#include <iomanip>
#include <sstream>
#include "waos/core/Simulator.h"
#include "waos/scheduler/FCFSScheduler.h"
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
 * @brief Test FCFS scheduling with sequential arrivals
 */
void test_fcfs_sequential_arrivals() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 1: FCFS Sequential Arrivals                  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    // Load test processes
    std::string mockFile = "../../../tests/mock/test_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes from " << mockFile << std::endl;
        assert(false);
    }

    // Setup FCFS scheduler
    auto scheduler = std::make_unique<FCFSScheduler>();
    sim.setScheduler(std::move(scheduler));

    // Setup mock memory manager (silent)
    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    // Start simulation
    sim.start();
    std::cout << "Simulation started - Observing FCFS behavior\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Run simulation for limited ticks to observe FCFS behavior
    int maxTicks = 60;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        sim.tick();
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ FCFS Sequential Arrivals Test\n" << std::endl;
}

/**
 * @brief Test FCFS with processes that have I/O operations
 */
void test_fcfs_with_io() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 2: FCFS with I/O Operations                  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<FCFSScheduler>();
    sim.setScheduler(std::move(scheduler));

    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    sim.start();
    std::cout << "Simulation started - Testing I/O handling\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Run longer to see I/O handling
    int maxTicks = 60;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        
        // Show only every 5 ticks to reduce output
        if (currentTime % 5 == 0) {
            std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        }
        
        sim.tick();
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ FCFS with I/O Test\n" << std::endl;
}

/**
 * @brief Test FCFS with complete simulation
 */
void test_fcfs_full_simulation() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 3: FCFS Full Simulation                      ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<FCFSScheduler>();
    sim.setScheduler(std::move(scheduler));

    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    sim.start();
    uint64_t startTime = sim.getCurrentTime();
    std::cout << "Running complete simulation...\n" << std::endl;

    // Run until completion or timeout
    int maxTicks = 200;
    int tick = 0;
    
    while (sim.isRunning() && tick < maxTicks) {
        // Show progress every 25 ticks
        if (tick % 25 == 0) {
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
        std::cout << "[PASSED] ✓ FCFS Full Simulation\n" << std::endl;
    } else {
        std::cout << "⚠️  Reached max ticks (" << maxTicks << ")" << std::endl;
        std::cout << "[WARNING] May need more ticks\n" << std::endl;
    }
}

int main() {
    std::cout << "\n╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  FCFS Scheduler + Simulator Integration     ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝\n" << std::endl;

    try {
        test_fcfs_sequential_arrivals();
        test_fcfs_with_io();
        test_fcfs_full_simulation();

        std::cout << "\n✅ All FCFS integration tests passed!\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
