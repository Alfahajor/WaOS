#include "waos/core/Process.h"
#include <stdexcept>
#include <random>
#include <iostream>

namespace waos::core {

  Process::Process(int pid, uint64_t arrivalTime, int priority, std::queue<Burst> bursts, int requiredPages)
    : m_pid(pid),
      m_priority(priority),
      m_state(ProcessState::NEW),
      m_arrivalTime(arrivalTime),
      m_bursts(std::move(bursts)),
      m_requiredPages(requiredPages),
      m_instructionPointer(0),
      m_quantumUsed(0),
      m_running(false),
      m_tickCompleted(false),
      m_stopThread(false) {

    if (m_pid < 0) throw std::invalid_argument("Process ID cannot be negative.");
    if (m_requiredPages < 0) throw std::invalid_argument("Required pages cannot be negative.");

    generateReferenceString();
  }

  Process::~Process() {
    stopThread();
  }

  void Process::startThread() {
    if (!m_thread.joinable()) {
      m_thread = std::thread(&Process::run, this);
    }
  }

  void Process::stopThread() {
    m_stopThread = true;
    {
      std::lock_guard<std::mutex> lock(m_processMutex);
      m_running = true; // Wake up if sleeping
    }
    m_cvRun.notify_one();
    
    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

  void Process::signalRun() {
    {
      std::lock_guard<std::mutex> lock(m_processMutex);
      m_running = true;
      m_tickCompleted = false; // Reset completion flag
    }
    m_cvRun.notify_one();
  }

  void Process::waitForTickCompletion() {
    std::unique_lock<std::mutex> lock(m_processMutex);
    m_cvKernel.wait(lock, [this] { return m_tickCompleted; });
    // Tick is done, Kernel can proceed
  }

  bool Process::simulateIoWait(int ticks) {
    std::lock_guard<std::mutex> lock(m_processMutex);
    
    if (m_bursts.empty()) return true;
    
    Burst& current = m_bursts.front();
    
    // Solo actuar si es una ráfaga de E/S
    if (current.type == BurstType::IO) {
      current.duration -= ticks;
      if (current.duration < 0) current.duration = 0;
      
      // Retorna true si el temporizador llegó a 0 (E/S completada)
      return current.duration == 0;
    }
    
    return false; // No era E/S o no terminó
  }

  void Process::run() {
    while (!m_stopThread) {
      // Wait for Kernel signal (Context Switch In)
      std::unique_lock<std::mutex> lock(m_processMutex);
      m_cvRun.wait(lock, [this] { return m_running || m_stopThread; });

      if (m_stopThread) break;

      // Execute one unit of work (CPU Tick)
      executeOneTick();

      // Notify Kernel (Context Switch Out / Yield)
      m_running = false; // Go back to sleep next iteration
      m_tickCompleted = true;
      lock.unlock(); // Unlock before notify to avoid pessimistic locking
      m_cvKernel.notify_one();
    }
  }

  bool Process::executeOneTick() {
    // Assumes lock is held by run()
    if (m_bursts.empty()) return true;

    Burst& current = m_bursts.front();
    
    if (current.type == BurstType::CPU) {
      current.duration--;
      if (current.duration < 0) current.duration = 0;
      advanceInstructionPointer();
    }

    return current.duration == 0;
  }

  int Process::getPid() const { return m_pid; }
  uint64_t Process::getArrivalTime() const { return m_arrivalTime; }
  int Process::getRequiredPages() const { return m_requiredPages; }
  int Process::getPriority() const { return m_priority; }

  ProcessStats Process::getStats() const {
    std::lock_guard<std::mutex> lock(m_processMutex);
    return m_stats;
  }

  ProcessState Process::getState() const { return m_state.load(); }

  void Process::setState(ProcessState newState, uint64_t currentTime) {
    std::lock_guard<std::mutex> lock(m_processMutex);
    ProcessState oldState = m_state.load();

    if (oldState == newState) return; // No change

    // Stats logic needs mutex protection now
    if (oldState == ProcessState::READY) {
      m_stats.totalWaitTime += (currentTime - m_stats.lastReadyTime);
    }

    m_state.store(newState);

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

  BurstType Process::getCurrentBurstType() const {
    std::lock_guard<std::mutex> lock(m_processMutex);
    if (m_bursts.empty()) return BurstType::CPU;
    return m_bursts.front().type;
  }

  int Process::getCurrentBurstDuration() const {
    std::lock_guard<std::mutex> lock(m_processMutex);
    if (m_bursts.empty()) return 0;
    return m_bursts.front().duration;
  }

  void Process::advanceToNextBurst() {
    std::lock_guard<std::mutex> lock(m_processMutex);
    if (!m_bursts.empty()) m_bursts.pop();
  }

  bool Process::hasMoreBursts() const {
    std::lock_guard<std::mutex> lock(m_processMutex);
    return !m_bursts.empty();
  }

  void Process::generateReferenceString() {
    // Generation happens in constructor (single thread context), no lock needed yet
    int totalCpuTicks = 0;
    // Temporal copy to iterate without destroying 
    std::queue<Burst> tempQueue = m_bursts;
    while(!tempQueue.empty()) {
      Burst b = tempQueue.front();
      if (b.type == BurstType::CPU) totalCpuTicks += b.duration;
      tempQueue.pop();
    }

    // Generate references using the Locality Principle
    // Deterministic seed based on PID for reproducibility
    std::mt19937 gen(m_pid); 
    std::uniform_int_distribution<> distPage(0, m_requiredPages - 1);
    std::uniform_real_distribution<> distProb(0.0, 1.0);

    int currentPage = 0;

    for (int i = 0; i < totalCpuTicks; ++i) {
      double p = distProb(gen);
      
      if (p < 0.7) { 
        // 70% de probabilidad de seguir en la misma página (Localidad espacial)
        m_pageReferenceString.push_back(currentPage);
      } else {
        // 30% de probabilidad de saltar a otra página aleatoria
        currentPage = distPage(gen);
        m_pageReferenceString.push_back(currentPage);
      }
    }
  }

  int Process::getCurrentPageRequirement() const {
    // No lock needed if m_instructionPointer is only touched by owned thread or during init
    // But for safety in getters:
    std::lock_guard<std::mutex> lock(m_processMutex);
    if (m_instructionPointer < m_pageReferenceString.size()) {
      return m_pageReferenceString[m_instructionPointer];
    }
    return 0; // Safe fallback
  }

  void Process::advanceInstructionPointer() {
    // Called internally by executeOneTick (already locked)
    if (m_instructionPointer < m_pageReferenceString.size()) {
      m_instructionPointer++;
    }
  }

  const std::vector<int>& Process::getPageReferenceString() const {
    return m_pageReferenceString;
  }

  int Process::getQuantumUsed() const {
    // Atomic read could be enough, but using mutex for consistency
    std::lock_guard<std::mutex> lock(m_processMutex);
    return m_quantumUsed;
  }

  void Process::resetQuantum() {
    std::lock_guard<std::mutex> lock(m_processMutex);
    m_quantumUsed = 0;
  }

  void Process::incrementQuantum(int ticks) {
    std::lock_guard<std::mutex> lock(m_processMutex);
    m_quantumUsed += ticks;
  }

  void Process::addCpuTime(uint64_t time) {
    std::lock_guard<std::mutex> lock(m_processMutex);
    m_stats.totalCpuTime += time;
  }

  void Process::addIoTime(uint64_t time) {
    std::lock_guard<std::mutex> lock(m_processMutex);
    m_stats.totalIoTime += time;
  }

  void Process::incrementPageFaults() {
    std::lock_guard<std::mutex> lock(m_processMutex);
    m_stats.pageFaults++;
  }

  void Process::incrementPreemptions() {
    std::lock_guard<std::mutex> lock(m_processMutex);
    m_stats.preemptions++;
  }

}
