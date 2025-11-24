#include "waos/scheduler/SJFScheduler.h"
#include "waos/core/Process.h"
#include <iostream>
#include <algorithm>

namespace waos::scheduler {

void SJFScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    m_pool.push_back(p);

    // Sort logic: Shortest Current Burst first
    std::sort(m_pool.begin(), m_pool.end(),
        [](waos::core::Process* a, waos::core::Process* b) {
            return a->getCurrentBurstDuration() < b->getCurrentBurstDuration();
        });
}


waos::core::Process* SJFScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pool.empty()) return nullptr;

    waos::core::Process* p = m_pool.front();
    m_pool.erase(m_pool.begin());
    return p;
}

bool SJFScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_pool.empty();
}

int SJFScheduler::getTimeSlice() const {
    return -1; // Non-preemptive by time (Preemptive version is SRTF, distinct usually)
}

}
