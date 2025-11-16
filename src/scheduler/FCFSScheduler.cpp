#include "waos/scheduler/FCFSScheduler.h"
#include "waos/core/Process.h"
#include <iostream>

namespace waos::scheduler {

void FCFSScheduler::addProcess(waos::core::Process* p) {
    if (!p) return;
    m_queue.push(p);
    std::cout << "[FCFS] addProcess PID=" << p->getPid() << "\n";
}
waos::core::Process* FCFSScheduler::getNextProcess() {
    if (m_queue.empty()) return nullptr;
    waos::core::Process* p = m_queue.front();
    m_queue.pop();
    std::cout << "[FCFS] getNextProcess PID=" << p->getPid() << "\n";
    return p;
}
bool FCFSScheduler::hasReadyProcesses() const {
    return !m_queue.empty();
}
} // namespace waos::scheduler