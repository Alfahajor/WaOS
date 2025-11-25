#include "waos/memory/OptimalMemoryManager.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace waos::memory {

  OptimalMemoryManager::OptimalMemoryManager(int totalFrames, const uint64_t* clockRef)
    : m_frames(totalFrames),
      m_clockRef(clockRef),
      m_pageFaults(0),
      m_pageReplacements(0) {
    if (totalFrames <= 0) throw std::invalid_argument("Total frames must be positive");
    if (!clockRef) throw std::invalid_argument("Clock reference cannot be null");
  }

  bool OptimalMemoryManager::isPageLoaded(int processId, int pageNumber) const {
    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) return false;

    const PageTable& pageTable = it->second;
    auto pageIt = pageTable.find(pageNumber);
    if (pageIt == pageTable.end()) return false;

    return pageIt->second.isLoaded();
  }

  PageRequestResult OptimalMemoryManager::requestPage(int processId, int pageNumber) {
    if (isPageLoaded(processId, pageNumber)) return PageRequestResult::HIT;

    m_pageFaults++;

    int frameIndex = findFreeFrame();
    if (frameIndex != -1) {
      loadPageIntoFrame(processId, pageNumber, frameIndex);
      return PageRequestResult::PAGE_FAULT;
    }

    // No free frames, use Optimal algorithm to select victim
    frameIndex = selectVictimFrame();
    evictFrame(frameIndex);
    loadPageIntoFrame(processId, pageNumber, frameIndex);
    m_pageReplacements++;
    
    return PageRequestResult::REPLACEMENT;
  }

  void OptimalMemoryManager::allocateForProcess(int processId, int requiredPages) {
    if (m_pageTables.find(processId) != m_pageTables.end()) return;

    PageTable pageTable;
    for (int i = 0; i < requiredPages; ++i) {
      PageTableEntry entry;
      entry.frameNumber = -1;
      entry.present = false;
      pageTable[i] = entry;
    }

    m_pageTables[processId] = pageTable;
  }

  void OptimalMemoryManager::freeForProcess(int processId) {
    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) return;

    for (Frame& frame : m_frames) {
      if (frame.pid == processId) frame.reset();
    }

    m_futureRefs.erase(processId);
    m_pageTables.erase(it);
  }

  void OptimalMemoryManager::completePageLoad(int processId, int pageNumber) {
    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) return;

    auto pageIt = it->second.find(pageNumber);
    if (pageIt != it->second.end() && pageIt->second.present) {
      pageIt->second.lastAccess = *m_clockRef;
    }
  }

  void OptimalMemoryManager::registerFutureReferences(int processId, 
                                                       const std::vector<int>& referenceString) {
    ProcessFutureReferences refs;
    refs.processId = processId;
    refs.futurePages = referenceString;
    refs.currentIndex = 0;

    m_futureRefs[processId] = refs;
  }

  void OptimalMemoryManager::advanceInstructionPointer(int processId) {
    auto it = m_futureRefs.find(processId);
    if (it != m_futureRefs.end()) {
      if (it->second.currentIndex < it->second.futurePages.size()) {
        it->second.currentIndex++;
      }
    }
  }

  int OptimalMemoryManager::getFreeFrames() const {
    return std::count_if(m_frames.begin(), m_frames.end(),
                         [](const Frame& f) { return f.isFree(); });
  }

  int OptimalMemoryManager::getActivePages(int processId) const {
    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) {
      return 0;
    }

    const PageTable& pageTable = it->second;
    return std::count_if(pageTable.begin(), pageTable.end(),
                         [](const auto& pair) { return pair.second.isLoaded(); });
  }

  int OptimalMemoryManager::findFreeFrame() const {
    for (size_t i = 0; i < m_frames.size(); ++i) {
      if (m_frames[i].isFree()) return static_cast<int>(i);
    }
    return -1;
  }

  void OptimalMemoryManager::loadPageIntoFrame(int processId, int pageNumber, int frameIndex) {
    // Update physical frame
    Frame& frame = m_frames[frameIndex];
    frame.pid = processId;
    frame.pageNumber = pageNumber;
    frame.occupied = true;
    frame.loadTime = *m_clockRef;
    frame.lastAccessTime = *m_clockRef;

    // Update page table entry
    PageTableEntry& entry = m_pageTables[processId][pageNumber];
    entry.load(frameIndex, *m_clockRef);
  }

  int OptimalMemoryManager::selectVictimFrame() {
    int victimFrame = 0;
    int maxDistance = -1;

    for (size_t i = 0; i < m_frames.size(); ++i) {
      if (!m_frames[i].occupied) continue;

      int distance = getNextUseDistance(m_frames[i].pid, m_frames[i].pageNumber);
      if (distance > maxDistance) {
        maxDistance = distance;
        victimFrame = static_cast<int>(i);
      }
    }

    return victimFrame;
  }

  void OptimalMemoryManager::evictFrame(int frameIndex) {
    Frame& frame = m_frames[frameIndex];
    if (!frame.occupied) return;

    PageTableEntry& entry = m_pageTables[frame.pid][frame.pageNumber];
    entry.evict();
  }

  int OptimalMemoryManager::getNextUseDistance(int processId, int pageNumber) const {
    auto it = m_futureRefs.find(processId);
    if (it == m_futureRefs.end()) return std::numeric_limits<int>::max();

    const ProcessFutureReferences& refs = it->second;
    size_t currentPos = refs.currentIndex;

    // Search forward from current position
    for (size_t i = currentPos; i < refs.futurePages.size(); ++i) {
      if (refs.futurePages[i] == pageNumber) {
        return static_cast<int>(i - currentPos);
      }
    }

    // Page never used again - optimal victim
    return std::numeric_limits<int>::max();
  }

}
