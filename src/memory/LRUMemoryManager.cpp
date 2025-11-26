#include "waos/memory/LRUMemoryManager.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace waos::memory {

  LRUMemoryManager::LRUMemoryManager(int totalFrames, const uint64_t* clockRef)
    : m_frames(totalFrames),
      m_clockRef(clockRef),
      m_pageFaults(0),
      m_pageReplacements(0) {
    if (totalFrames <= 0) throw std::invalid_argument("Total frames must be positive");
    if (!clockRef) throw std::invalid_argument("Clock reference cannot be null");
  }

  bool LRUMemoryManager::isPageLoaded(int processId, int pageNumber) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) return false;

    const PageTable& pageTable = it->second;
    auto pageIt = pageTable.find(pageNumber);
    if (pageIt == pageTable.end()) return false;

    return pageIt->second.isLoaded();
  }

  PageRequestResult LRUMemoryManager::requestPage(int processId, int pageNumber) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Page hit - update access time for LRU
    if (isPageLoaded(processId, pageNumber)) {
      updateAccessTime(processId, pageNumber);
      return PageRequestResult::HIT;
    }

    m_pageFaults++;

    // Try to find a free frame
    int frameIndex = findFreeFrame();
    if (frameIndex != -1) {
      loadPageIntoFrame(processId, pageNumber, frameIndex);
      return PageRequestResult::PAGE_FAULT;
    }

    // No free frames, must replace using LRU
    frameIndex = selectVictimFrame();
    evictFrame(frameIndex);
    loadPageIntoFrame(processId, pageNumber, frameIndex);
    m_pageReplacements++;
    
    return PageRequestResult::REPLACEMENT;
  }

  void LRUMemoryManager::allocateForProcess(int processId, int requiredPages) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_pageTables.find(processId) != m_pageTables.end()) return;

    PageTable pageTable;
    for (int i = 0; i < requiredPages; ++i) {
      PageTableEntry entry;
      entry.frameNumber = -1;
      entry.present = false;
      entry.lastAccess = 0;
      pageTable[i] = entry;
    }

    m_pageTables[processId] = pageTable;
  }

  void LRUMemoryManager::freeForProcess(int processId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) return;

    for (Frame& frame : m_frames) {
      if (frame.pid == processId) frame.reset();
    }

    m_pageTables.erase(it);
  }

  void LRUMemoryManager::completePageLoad(int processId, int pageNumber) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) return;

    auto pageIt = it->second.find(pageNumber);
    if (pageIt == it->second.end() || !pageIt->second.present) return;

    pageIt->second.lastAccess = *m_clockRef;
    
    int frameIndex = pageIt->second.frameNumber;
    if (frameIndex >= 0 && frameIndex < static_cast<int>(m_frames.size())) {
      m_frames[frameIndex].lastAccessTime = *m_clockRef;
    }
  }

  int LRUMemoryManager::getFreeFrames() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    return std::count_if(m_frames.begin(), m_frames.end(),
                         [](const Frame& f) { return f.isFree(); });
  }

  uint64_t LRUMemoryManager::getPageFaults() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_pageFaults;
  }

  uint64_t LRUMemoryManager::getPageReplacements() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_pageReplacements;
  }

  int LRUMemoryManager::getActivePages(int processId) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) {
      return 0;
    }

    const PageTable& pageTable = it->second;
    return std::count_if(pageTable.begin(), pageTable.end(),
                         [](const auto& pair) { return pair.second.isLoaded(); });
  }

  int LRUMemoryManager::findFreeFrame() const {
    for (size_t i = 0; i < m_frames.size(); ++i) {
      if (m_frames[i].isFree()) return static_cast<int>(i);
    }
    return -1;
  }

  void LRUMemoryManager::loadPageIntoFrame(int processId, int pageNumber, int frameIndex) {
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

  int LRUMemoryManager::selectVictimFrame() {
    uint64_t oldestTime = std::numeric_limits<uint64_t>::max();
    int victimFrame = 0;

    for (size_t i = 0; i < m_frames.size(); ++i) {
      if (m_frames[i].occupied && m_frames[i].lastAccessTime < oldestTime) {
        oldestTime = m_frames[i].lastAccessTime;
        victimFrame = static_cast<int>(i);
      }
    }

    return victimFrame;
  }

  void LRUMemoryManager::evictFrame(int frameIndex) {
    Frame& frame = m_frames[frameIndex];
    if (!frame.occupied) return;

    PageTableEntry& entry = m_pageTables[frame.pid][frame.pageNumber];
    entry.evict();
  }

  void LRUMemoryManager::updateAccessTime(int processId, int pageNumber) {
    PageTableEntry& entry = m_pageTables[processId][pageNumber];
    entry.lastAccess = *m_clockRef;

    int frameIndex = entry.frameNumber;
    if (frameIndex >= 0 && frameIndex < static_cast<int>(m_frames.size())) {
      m_frames[frameIndex].lastAccessTime = *m_clockRef;
    }
  }

}
