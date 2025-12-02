#include "waos/scheduler/SJFScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

// Comparator implementation: returns true if 'a' has LOWER priority than 'b'
// (priority_queue is a max-heap by default, so we invert to get min-heap behavior)
bool SJFScheduler::ProcessComparator::operator()(waos::core::Process* a, waos::core::Process* b) const {
    // Return true if 'a' should come AFTER 'b' in the queue (a has longer burst)
    // This makes processes with SHORTER bursts have HIGHER priority
    return a->getCurrentBurstDuration() > b->getCurrentBurstDuration();
}

SJFScheduler::SJFScheduler() {
    m_metrics.totalSchedulingDecisions = 0;
}

void SJFScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    // O(log n) insertion into priority queue (min-heap by burst duration)
    m_priorityQueue.push(p);
    
    std::cout << "  [SJF] Added P" << p->getPid() 
              << " (burst=" << p->getCurrentBurstDuration() << ") to ready queue" << std::endl;
}


waos::core::Process* SJFScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_priorityQueue.empty()) return nullptr;

    // O(log n) extraction of process with minimum burst duration
    waos::core::Process* p = m_priorityQueue.top();
    m_priorityQueue.pop();

    m_metrics.totalSchedulingDecisions++;
    m_metrics.selectionCount[p->getPid()]++;

    std::cout << "  [SJF] Selected P" << p->getPid() 
              << " for execution (shortest burst=" << p->getCurrentBurstDuration() << ")" << std::endl;
    return p;
}

bool SJFScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_priorityQueue.empty();
}

int SJFScheduler::getTimeSlice() const {
    return -1; // Non-preemptive by time (Preemptive version is SRTF, distinct usually)
}

std::vector<const waos::core::Process*> SJFScheduler::peekReadyQueue() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Copy priority queue to vector for observation
    // Note: This requires extracting all elements - O(n log n) operation
    // This is acceptable since peekReadyQueue is used infrequently (only for debugging/monitoring)
    auto temp_pq = m_priorityQueue;
    std::vector<const waos::core::Process*> result;
    
    while (!temp_pq.empty()) {
        result.push_back(temp_pq.top());
        temp_pq.pop();
    }
    
    return result;
}

std::string SJFScheduler::getAlgorithmName() const {
    return "SJF (Shortest Job First - Priority Queue)";
}

waos::common::SchedulerMetrics SJFScheduler::getSchedulerMetrics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_metrics;
}

}
