#include "waos/scheduler/SJFScheduler.h"
#include "waos/core/Process.h"
#include <iostream>
#include <algorithm>

namespace waos::scheduler {

void SJFScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pool.push_back(p);
    std::cout << "[SJF] addProcess PID=" << p->getPid()
              << " burstDuration=" << p->getCurrentBurstDuration() << "\n";
}

waos::core::Process* SJFScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pool.empty()) return nullptr;
    
    // Find the process with the shortest CPU burst
    auto it = std::min_element(m_pool.begin(), m_pool.end(),
        [](waos::core::Process* a, waos::core::Process* b) {
            return a->getCurrentBurstDuration() < b->getCurrentBurstDuration();
        });
    
    waos::core::Process* p = *it;
    m_pool.erase(it);
    
    std::cout << "[SJF] getNextProcess PID=" << p->getPid()
              << " burstDuration=" << p->getCurrentBurstDuration() << "\n";
    return p;
}

bool SJFScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_pool.empty();
}

} // namespace waos::scheduler
