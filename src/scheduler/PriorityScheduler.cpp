#include "waos/scheduler/PriorityScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

void PriorityScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    // Currently Process has no priority getter; default to 0.
    //The process still needs to be extended to have priority.
    // Future: if Process defines getPriority(), use that.
    int priority = 0;

    m_queues[priority].push_back(p);
    std::cout << "[Priority-stub] addProcess PID=" << p->getPid()
              << " priority=" << priority << "\n";
}

waos::core::Process* PriorityScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queues.empty()) return nullptr;

    // find first non-empty queue starting from lowest priority key (0 = highest)
    for (auto it = m_queues.begin(); it != m_queues.end(); ++it) {
        if (!it->second.empty()) {
            waos::core::Process* p = it->second.front();
            it->second.pop_front();
            std::cout << "[Priority-stub] getNextProcess PID=" << p->getPid()
                      << " priority=" << it->first << "\n";
            return p;
        }
    }
    return nullptr;
}

bool PriorityScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &kv : m_queues) {
        if (!kv.second.empty()) return true;
    }
    return false;
}

} // namespace waos::scheduler
