#include "waos/scheduler/FCFSScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

void FCFSScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(p);
    std::cout << "[FCFS] addProcess PID=" << p->getPid() << "\n";
}

waos::core::Process* FCFSScheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return nullptr;
    waos::core::Process* p = m_queue.front();
    m_queue.pop();
    std::cout << "[FCFS] getNextProcess PID=" << p->getPid() << "\n";
    return p;
}

bool FCFSScheduler::hasReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_queue.empty();
}

} // namespace waos::scheduler