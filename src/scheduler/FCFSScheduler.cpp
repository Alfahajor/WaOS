#include "waos/scheduler/FCFSScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

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

} // namespace waos::scheduler
