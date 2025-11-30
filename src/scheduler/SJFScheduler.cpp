#include "waos/scheduler/SJFScheduler.h"
#include "waos/core/Process.h"
#include <iostream>
#include <algorithm>

namespace waos::scheduler {

SJFScheduler::SJFScheduler() {
    m_metrics.totalSchedulingDecisions = 0;
}

void SJFScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    m_pool.push_back(p);

    // Sort logic: Shortest Current Burst first
    std::sort(m_pool.begin(), m_pool.end(),
        [](waos::core::Process* a, waos::core::Process* b) {
            return a->getCurrentBurstDuration() < b->getCurrentBurstDuration();
        });
    
    std::cout << "  [SJF] Added P" << p->getPid() 
              << " (burst=" << p->getCurrentBurstDuration() << ") to ready queue" << std::endl;
}


waos::core::Process* SJFScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pool.empty()) return nullptr;

    waos::core::Process* p = m_pool.front();
    m_pool.erase(m_pool.begin());

    m_metrics.totalSchedulingDecisions++;
    m_metrics.selectionCount[p->getPid()]++;

    std::cout << "  [SJF] Selected P" << p->getPid() 
              << " for execution (shortest burst=" << p->getCurrentBurstDuration() << ")" << std::endl;
    return p;
}

bool SJFScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_pool.empty();
}

int SJFScheduler::getTimeSlice() const {
    return -1; // Non-preemptive by time (Preemptive version is SRTF, distinct usually)
}

std::vector<const waos::core::Process*> SJFScheduler::peekReadyQueue() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Vector directo, fácil de convertir
    return std::vector<const waos::core::Process*>(m_pool.begin(), m_pool.end());
}

std::string SJFScheduler::getAlgorithmName() const {
    return "SJF (Shortest Job First)";
}

waos::common::SchedulerMetrics SJFScheduler::getSchedulerMetrics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_metrics;
}

}
