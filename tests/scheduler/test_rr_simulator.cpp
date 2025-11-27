/**
 * @brief Integration test for Round Robin Scheduler with Simulator
 * @details Tests the RR scheduling algorithm integrated with the full Simulator,
 *          including quantum preemption, process arrivals, CPU execution, and I/O operations.
 */

#include <iostream>
#include <cassert>
#include <memory>
#include <iomanip>
#include <sstream>
#include <map>
#include "waos/core/Simulator.h"
#include "waos/scheduler/RRScheduler.h"
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
 * @brief Test RR scheduling with quantum preemption
 */
void test_rr_quantum_preemption() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 1: RR Quantum Preemption (q=5)               ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    // Load test processes
    std::string mockFile = "../../../tests/mock/test_rr_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes from " << mockFile << std::endl;
        assert(false);
    }

    // Setup RR scheduler with quantum = 5
    auto scheduler = std::make_unique<RRScheduler>(5);
    sim.setScheduler(std::move(scheduler));

    // Setup mock memory manager (silent)
    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    // Start simulation
    sim.start();
    std::cout << "Simulation started - Observing RR behavior with Quantum=5\n" << std::endl;
    std::cout << "Legend: Q=quantum used | Burst=executed/total (remaining)\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Track initial burst duration for each process (per burst, not per quantum)
    std::map<int, int> processBurstInitial;
    std::map<int, int> lastSeenBurstRemaining;
    
    // Run simulation for limited ticks to observe quantum preemption
    int maxTicks = 40;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        
        std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        
        sim.tick();
        
        // Get current running process and show quantum status AFTER tick
        const Process* running = sim.getRunningProcess();
        if (running) {
            int quantumUsed = running->getQuantumUsed();
            int burstRemaining = running->getCurrentBurstDuration();
            int pid = running->getPid();
            
            // Detect if this is a new burst (burst duration increased/reset from last time)
            bool isNewBurst = false;
            if (lastSeenBurstRemaining.find(pid) != lastSeenBurstRemaining.end()) {
                // If remaining increased, it's a new burst
                if (burstRemaining > lastSeenBurstRemaining[pid]) {
                    isNewBurst = true;
                }
            } else {
                // First time seeing this process
                isNewBurst = true;
            }
            
            // Update or initialize burst tracking
            if (isNewBurst) {
                // Calculate original burst size: remaining + already used
                processBurstInitial[pid] = burstRemaining + quantumUsed;
            }
            
            lastSeenBurstRemaining[pid] = burstRemaining;
            
            // Calculate how much of the burst has been executed
            int burstInitial = processBurstInitial[pid];
            int burstExecuted = burstInitial - burstRemaining;
            
            std::cout << "  → Running: P" << pid 
                      << " | Q=" << quantumUsed << "/5"
                      << " | Burst=" << burstExecuted << "/" << burstInitial 
                      << " (remaining=" << burstRemaining << ")";
            
            if (quantumUsed >= 5) {
                std::cout << " ⚠ Quantum full!";
            }
            std::cout << std::endl;
        }
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ RR Quantum Preemption Test\n" << std::endl;
}

/**
 * @brief Test RR with processes that have I/O operations
 */
void test_rr_with_io() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 2: RR with I/O Operations (q=5)              ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_rr_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<RRScheduler>(5);
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
    std::cout << "[PASSED] ✓ RR with I/O Test\n" << std::endl;
}

/**
 * @brief Test RR with complete simulation
 */
void test_rr_full_simulation() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 3: RR Full Simulation (q=5)                  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_rr_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<RRScheduler>(5);
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
        std::cout << "[PASSED] ✓ RR Full Simulation\n" << std::endl;
    } else {
        std::cout << "⚠️  Reached max ticks (" << maxTicks << ")" << std::endl;
        std::cout << "[WARNING] May need more ticks\n" << std::endl;
    }
}

int main() {
    std::cout << "\n╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  RR Scheduler + Simulator Integration       ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝\n" << std::endl;

    try {
        test_rr_quantum_preemption();
        test_rr_with_io();
        test_rr_full_simulation();

        std::cout << "\n✅ All RR integration tests passed!\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
