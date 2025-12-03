#include "waos/memory/FIFOMemoryManager.h"

#include <algorithm>
#include <stdexcept>

namespace waos::memory {

FIFOMemoryManager::FIFOMemoryManager(int totalFrames, const uint64_t* clockRef)
    : m_frames(totalFrames), m_clockRef(clockRef) {
  m_stats.totalFrames = totalFrames;
  m_stats.usedFrames = 0;
  m_stats.totalPageFaults = 0;
  m_stats.totalReplacements = 0;
  m_stats.hitRatio = 0.0;

  if (totalFrames <= 0) throw std::invalid_argument("Total frames must be positive");
  if (!clockRef) throw std::invalid_argument("Clock reference cannot be null");
}

bool FIFOMemoryManager::isPageLoaded(int processId, int pageNumber) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_pageTables.find(processId);
  if (it == m_pageTables.end()) return false;

  const PageTable& pageTable = it->second;
  auto pageIt = pageTable.find(pageNumber);
  if (pageIt == pageTable.end()) return false;

  return pageIt->second.isLoaded();
}

PageRequestResult FIFOMemoryManager::requestPage(int processId, int pageNumber) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Page already loaded (Inline check to avoid recursive lock)
  auto it = m_pageTables.find(processId);
  if (it != m_pageTables.end()) {
    auto pageIt = it->second.find(pageNumber);
    if (pageIt != it->second.end() && pageIt->second.isLoaded()) {
      m_totalHits++;  // Count the hit
      return PageRequestResult::HIT;
    }
  }

  m_stats.totalPageFaults++;
  m_stats.faultsPerProcess[processId]++;

  // Try to find a free frame
  int frameIndex = findFreeFrame();
  if (frameIndex != -1) {
    loadPageIntoFrame(processId, pageNumber, frameIndex);
    m_loadQueue.push({processId, pageNumber});
    return PageRequestResult::PAGE_FAULT;
  }

  // No free frames, must replace using FIFO
  frameIndex = selectVictimFrame();
  evictFrame(frameIndex);
  loadPageIntoFrame(processId, pageNumber, frameIndex);
  m_loadQueue.push({processId, pageNumber});
  m_stats.totalReplacements++;

  return PageRequestResult::REPLACEMENT;
}

void FIFOMemoryManager::allocateForProcess(int processId, int requiredPages) {
  std::lock_guard<std::mutex> lock(m_mutex);

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

void FIFOMemoryManager::freeForProcess(int processId) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_pageTables.find(processId);
  if (it == m_pageTables.end()) return;

  // Free all frames used by this process
  for (Frame& frame : m_frames) {
    if (frame.pid == processId) {
      frame.reset();
      m_stats.usedFrames--;
    }
  }

  // Remove from FIFO queue
  std::queue<std::pair<int, int>> tempQueue;
  while (!m_loadQueue.empty()) {
    auto item = m_loadQueue.front();
    m_loadQueue.pop();
    if (item.first != processId) tempQueue.push(item);
  }
  m_loadQueue = tempQueue;

  m_pageTables.erase(it);
}

void FIFOMemoryManager::completePageLoad(int processId, int pageNumber) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_pageTables.find(processId);
  if (it == m_pageTables.end()) return;

  auto pageIt = it->second.find(pageNumber);
  if (pageIt != it->second.end() && pageIt->second.present) {
    pageIt->second.lastAccess = *m_clockRef;
  }
}

std::vector<waos::common::FrameInfo> FIFOMemoryManager::getFrameStatus() const {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::vector<waos::common::FrameInfo> result;
  for (int i = 0; i < m_frames.size(); ++i) {
    waos::common::FrameInfo info;
    info.frameId = i;
    info.isOccupied = m_frames[i].occupied;
    info.ownerPid = m_frames[i].pid;
    info.pageNumber = m_frames[i].pageNumber;
    info.loadedAtTick = m_frames[i].loadTime;
    result.push_back(info);
  }
  return result;
}

std::vector<waos::common::PageTableEntryInfo> FIFOMemoryManager::getPageTableForProcess(int processId) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::vector<waos::common::PageTableEntryInfo> result;
  auto it = m_pageTables.find(processId);
  if (it != m_pageTables.end()) {
    for (const auto& pair : it->second) {
      waos::common::PageTableEntryInfo info;
      info.pageNumber = pair.first;
      info.frameNumber = pair.second.frameNumber;
      info.present = pair.second.present;
      info.referenced = pair.second.referenced;
      info.modified = pair.second.modified;
      result.push_back(info);
    }
  }
  return result;
}

waos::common::MemoryStats FIFOMemoryManager::getMemoryStats() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  waos::common::MemoryStats currentStats = m_stats;
  uint64_t totalAccesses = m_stats.totalPageFaults + m_totalHits;
  currentStats.hitRatio = (totalAccesses > 0) ? (double)m_totalHits / totalAccesses * 100.0 : 0.0;
  return currentStats;
}

std::string FIFOMemoryManager::getAlgorithmName() const {
  return "FIFO (First-In, First-Out)";
}

int FIFOMemoryManager::findFreeFrame() const {
  for (size_t i = 0; i < m_frames.size(); ++i) {
    if (m_frames[i].isFree()) return static_cast<int>(i);
  }
  return -1;
}

void FIFOMemoryManager::loadPageIntoFrame(int processId, int pageNumber, int frameIndex) {
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
  m_stats.usedFrames++;
}

int FIFOMemoryManager::selectVictimFrame() {
  if (m_loadQueue.empty()) {
    // Fallback: return first occupied frame
    for (size_t i = 0; i < m_frames.size(); ++i) {
      if (m_frames[i].occupied) return static_cast<int>(i);
    }
    return 0;
  }

  auto oldest = m_loadQueue.front();
  m_loadQueue.pop();

  const PageTableEntry& entry = m_pageTables[oldest.first][oldest.second];
  return entry.frameNumber;
}

void FIFOMemoryManager::evictFrame(int frameIndex) {
  Frame& frame = m_frames[frameIndex];
  if (!frame.occupied) return;

  PageTableEntry& entry = m_pageTables[frame.pid][frame.pageNumber];
  entry.evict();
  m_stats.usedFrames--;
}

}  // namespace waos::memory
