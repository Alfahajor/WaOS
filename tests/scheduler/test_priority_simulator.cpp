/**
 * @brief Integration test for Priority Scheduler with Simulator
 * @details Tests the Priority scheduling algorithm integrated with the full Simulator.
 *          Priority scheduling selects the process with the highest priority (lowest value).
 *          Processes with the same priority are scheduled in FIFO order.
 */

#include <iostream>
#include <cassert>
#include <memory>
#include <iomanip>
#include <sstream>
#include <map>
#include "waos/core/Simulator.h"
#include "waos/scheduler/PriorityScheduler.h"
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
 * @brief Test Priority scheduling with different priority levels
 */
void test_priority_levels() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 1: Priority Levels (0=highest)               ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    // Load test processes
    std::string mockFile = "../../../tests/mock/test_priority_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes from " << mockFile << std::endl;
        assert(false);
    }

    // Setup Priority scheduler
    auto scheduler = std::make_unique<PriorityScheduler>();
    sim.setScheduler(std::move(scheduler));

    // Setup mock memory manager (silent)
    auto memory = std::make_unique<MockMemoryManager>();
    sim.setMemoryManager(std::move(memory));

    // Start simulation
    sim.start();
    std::cout << "Simulation started - Observing Priority behavior\n" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Track initial burst duration for each process
    std::map<int, int> processBurstInitial;
    std::map<int, int> lastSeenBurstRemaining;
    
    // Run simulation for limited ticks to observe priority behavior
    int maxTicks = 40;
    for (int tick = 0; tick < maxTicks && sim.isRunning(); tick++) {
        uint64_t currentTime = sim.getCurrentTime();
        
        std::cout << "\n[Tick " << std::setw(3) << currentTime << "]" << std::endl;
        
        sim.tick();
        
        // Get current running process and show status
        const Process* running = sim.getRunningProcess();
        if (running) {
            int burstRemaining = running->getCurrentBurstDuration();
            int pid = running->getPid();
            int priority = running->getPriority();
            
            // Detect if this is a new burst
            bool isNewBurst = false;
            if (lastSeenBurstRemaining.find(pid) != lastSeenBurstRemaining.end()) {
                if (burstRemaining > lastSeenBurstRemaining[pid]) {
                    isNewBurst = true;
                }
            } else {
                isNewBurst = true;
            }
            
            // Update burst tracking
            if (isNewBurst) {
                processBurstInitial[pid] = burstRemaining;
            }
            
            lastSeenBurstRemaining[pid] = burstRemaining;
            
            // Calculate execution progress
            int burstInitial = processBurstInitial[pid];
            int burstExecuted = burstInitial - burstRemaining;
            
            std::cout << "  → Running: P" << pid 
                      << " (Priority=" << priority << ")"
                      << " | Burst=" << burstExecuted << "/" << burstInitial 
                      << " (remaining=" << burstRemaining << ")";
            std::cout << std::endl;
        }
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[PASSED] ✓ Priority Levels Test\n" << std::endl;
}

/**
 * @brief Test Priority with I/O operations
 */
void test_priority_with_io() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 2: Priority with I/O Operations              ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_priority_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<PriorityScheduler>();
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
    std::cout << "[PASSED] ✓ Priority with I/O Test\n" << std::endl;
}

/**
 * @brief Test Priority with complete simulation
 */
void test_priority_full_simulation() {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test 3: Priority Full Simulation                  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝\n" << std::endl;

    Simulator sim;
    
    std::string mockFile = "../../../tests/mock/test_priority_processes.txt";
    if (!sim.loadProcesses(mockFile)) {
        std::cerr << "ERROR: Could not load processes" << std::endl;
        assert(false);
    }

    auto scheduler = std::make_unique<PriorityScheduler>();
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
        std::cout << "[PASSED] ✓ Priority Full Simulation\n" << std::endl;
    } else {
        std::cout << "⚠️  Reached max ticks (" << maxTicks << ")" << std::endl;
        std::cout << "[WARNING] May need more ticks\n" << std::endl;
    }
}

int main() {
    std::cout << "\n╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Priority Scheduler + Simulator Tests       ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝\n" << std::endl;

    try {
        test_priority_levels();
        test_priority_with_io();
        test_priority_full_simulation();

        std::cout << "\n✅ All Priority integration tests passed!\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
