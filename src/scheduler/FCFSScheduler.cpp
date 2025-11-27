#include "waos/scheduler/FCFSScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

FCFSScheduler::FCFSScheduler() {
    m_metrics.totalSchedulingDecisions = 0;
    m_metrics.totalPreemptions = 0;
}

void FCFSScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(p);
    std::cout << "  [FCFS] Added P" << p->getPid() << " to ready queue" << std::endl;
}

waos::core::Process* FCFSScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return nullptr;

    waos::core::Process* p = m_queue.front();
    m_queue.pop();

    m_metrics.totalSchedulingDecisions++;
    m_metrics.selectionCount[p->getPid()]++;

    std::cout << "  [FCFS] Selected P" << p->getPid() << " for execution (FIFO)" << std::endl;
    return p;
}

bool FCFSScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_queue.empty();
}

int FCFSScheduler::getTimeSlice() const {
    return -1; // Non-preemptive by time
}

std::vector<const waos::core::Process*> FCFSScheduler::peekReadyQueue() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<const waos::core::Process*> result;
    
    // std::queue no permite iterar, hacemos una copia temporal
    std::queue<waos::core::Process*> temp = m_queue;
    while (!temp.empty()) {
        result.push_back(temp.front());
        temp.pop();
    }
    return result;
}

std::string FCFSScheduler::getAlgorithmName() const {
    return "FCFS (First-Come, First-Served)";
}

waos::common::SchedulerMetrics FCFSScheduler::getSchedulerMetrics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_metrics;
}

} // namespace waos::scheduler
