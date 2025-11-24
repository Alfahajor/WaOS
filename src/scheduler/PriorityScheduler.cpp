#include "waos/scheduler/PriorityScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

void PriorityScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    // Currently Process has no priority getter
    int priority = p->getPriority();

    m_queues[priority].push_back(p);
}

waos::core::Process* PriorityScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queues.empty()) return nullptr;

    // Iterate from highest priority (lowest key)
    for (auto it = m_queues.begin(); it != m_queues.end(); ++it) {
        auto& q = it->second;
        if (!q.empty()) {
            waos::core::Process* p = q.front();
            q.pop_front();

            // Remove queue if now empty
            if (q.empty()) it = m_queues.erase(it);
            else ++it;

            return p;
        }
        it = m_queues.erase(it);
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

int PriorityScheduler::getTimeSlice() const {
    return -1; // Non-preemptive priority by default
}

}
