#include "waos/scheduler/RRScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

void RRScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(p);
    std::cout << "  [RR] Added P" << p->getPid() << " to ready queue (FIFO order)" << std::endl;
}

waos::core::Process* RRScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return nullptr;

    waos::core::Process* p = m_queue.front();
    m_queue.pop();
    
    std::cout << "  [RR] Selected P" << p->getPid() << " for execution (Quantum=" 
              << m_quantum << " ticks)" << std::endl;
    
    // Note: Process will be re-queued by the Simulator after quantum expiration
    // or I/O operation. The RR scheduler simply maintains FIFO order in the queue.
    // The quantum management is handled by the Simulator which will:
    // 1. Let the process run for up to m_quantum ticks
    // 2. Preempt it if quantum expires before burst completes
    // 3. Re-add the process to the scheduler if preempted
    
    return p;
}

bool RRScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_queue.empty();
}

int RRScheduler::getTimeSlice() const { 
    return m_quantum; 
}

}
