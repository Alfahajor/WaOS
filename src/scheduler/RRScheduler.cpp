#include "waos/scheduler/RRScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

void RRScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(p);
    std::cout << "[RR-stub] addProcess PID=" << p->getPid()
              << " quantum_ms=" << m_quantum.count() << "\n";
}

waos::core::Process* RRScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return nullptr;
    waos::core::Process* p = m_queue.front();
    m_queue.pop();
    // Stub: do not requeue. Real RR logic will requeue if not finished.
    // TODO: Implement proper Round Robin scheduling with:
    // - Process re-queueing after quantum expiration
    // - Quantum time management
    // - Process state transitions (READY ↔ RUNNING)
    // - Burst time tracking and completion detection
    
    std::cout << "[RR-stub] getNextProcess PID=" << p->getPid() 
              << " | WARNING: Process not re-queued (stub behavior)\n";
    std::cout << "[RR-stub] getNextProcess PID=" << p->getPid() << "\n";
    return p;
}

bool RRScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_queue.empty();
}

} // namespace waos::scheduler