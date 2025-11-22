#include "waos/core/Process.h"
#include <stdexcept>

namespace waos::core {

  Process::Process(int pid, uint64_t arrivalTime, std::queue<int> bursts, int requiredPages)
    : m_pid(pid),
      m_state(ProcessState::NEW),
      m_arrivalTime(arrivalTime),
      m_bursts(std::move(bursts)),
      m_requiredPages(requiredPages) {
    if (m_pid < 0) {
      throw std::invalid_argument("Process ID cannot be negative.");
    }
    if (m_requiredPages < 0) {
      throw std::invalid_argument("Required pages cannot be negative.");
    }
  }

  int Process::getPid() const { return m_pid; }

  uint64_t Process::getArrivalTime() const { return m_arrivalTime; }

  int Process::getRequiredPages() const { return m_requiredPages; }

  ProcessState Process::getState() const { return m_state; }

  void Process::setState(ProcessState newState, uint64_t currentTime) {
    if (m_state == newState) return; // No change

    // Handle statistics based on state transitions
    if (m_state == ProcessState::READY) {
      // Exiting READY state, so calculate wait time
      m_stats.totalWaitTime += (currentTime - m_stats.lastReadyTime);
    }

    m_state = newState;

    // Handle statistics for the new state
    if (newState == ProcessState::READY) {
      // Entering READY state, record the time
      m_stats.lastReadyTime = currentTime;
    } else if (newState == ProcessState::RUNNING && m_stats.startTime == 0) {
      // First time running
      m_stats.startTime = currentTime;
    } else if (newState == ProcessState::TERMINATED) {
      m_stats.finishTime = currentTime;
    }
  }

  int Process::getCurrentBurstType() const {
    if (m_bursts.empty()) return BurstType::CPU;
    return m_bursts.front().type;
  }

  int Process::getCurrentBurstDuration() const {
    if (m_bursts.empty()) return 0;
    return m_bursts.front().duration;
  }

  bool Process::updateCurrentBurst(int timeUnits) {
    if (m_bursts.empty()) return true;

    Burst& current = m_bursts.front();
    current.duration -= timeUnits;
    
    if (current.duration < 0) current.duration = 0;
    return current.duration == 0;
  }

  void Process::advanceToNextBurst() {
    if (!m_bursts.empty()) m_bursts.pop();
  }

  bool Process::hasMoreBursts() const {
    return !m_bursts.empty();
  }
  
  const ProcessStats& Process::getStats() const { return m_stats; }

  void Process::addCpuTime(uint64_t time) { m_stats.totalCpuTime += time; }
  void Process::addIoTime(uint64_t time) { m_stats.totalIoTime += time; }

}
