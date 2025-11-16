#include "waos/scheduler/SJFScheduler.h"
#include "waos/core/Process.h"
#include <iostream>
#include <algorithm>

namespace waos::scheduler {

void SJFScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pool.push_back(p);
    std::cout << "[SJF-stub] addProcess PID=" << p->getPid()
              << " arrival=" << p->getArrivalTime() << "\n";
}

waos::core::Process* SJFScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pool.empty()) return nullptr;
    
    //just for testing
    //this will be replaced with actual SJF logic
    
    // Stub behaviour: pick first-inserted. In future: pick smallest getCurrentCpuBurst()
    // Example future selection (commented):
    // auto it = std::min_element(m_pool.begin(), m_pool.end(),
    //     [](waos::core::Process* a, waos::core::Process* b){
    //         return a->getCurrentCpuBurst() < b->getCurrentCpuBurst();
    //     });
    // waos::core::Process* p = *it; m_pool.erase(it);

    waos::core::Process* p = m_pool.front();
    m_pool.erase(m_pool.begin());
    std::cout << "[SJF-stub] getNextProcess PID=" << p->getPid() << "\n";
    return p;
}

bool SJFScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_pool.empty();
}

} // namespace waos::scheduler
