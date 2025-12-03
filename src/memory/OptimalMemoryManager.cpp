#include "waos/memory/OptimalMemoryManager.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace waos::memory {

OptimalMemoryManager::OptimalMemoryManager(int totalFrames, const uint64_t* clockRef)
    : m_frames(totalFrames), m_clockRef(clockRef) {
  m_stats.totalFrames = totalFrames;
  m_stats.usedFrames = 0;
  m_stats.totalPageFaults = 0;
  m_stats.totalReplacements = 0;
  m_stats.hitRatio = 0.0;
  if (totalFrames <= 0) throw std::invalid_argument("Total frames must be positive");
  if (!clockRef) throw std::invalid_argument("Clock reference cannot be null");
}

bool OptimalMemoryManager::isPageLoaded(int processId, int pageNumber) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_pageTables.find(processId);
  if (it == m_pageTables.end()) return false;

  const PageTable& pageTable = it->second;
  auto pageIt = pageTable.find(pageNumber);
  if (pageIt == pageTable.end()) return false;

  return pageIt->second.isLoaded();
}

PageRequestResult OptimalMemoryManager::requestPage(int processId, int pageNumber) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_pageTables.count(processId) &&
      m_pageTables[processId].count(pageNumber) &&
      m_pageTables[processId][pageNumber].isLoaded()) {
    m_totalHits++;
    return PageRequestResult::HIT;
  }

  m_stats.totalPageFaults++;
  m_stats.faultsPerProcess[processId]++;

  int frameIndex = findFreeFrame();
  if (frameIndex != -1) {
    loadPageIntoFrame(processId, pageNumber, frameIndex);
    return PageRequestResult::PAGE_FAULT;
  }

  // No free frames, use Optimal algorithm to select victim
  frameIndex = selectVictimFrame();
  evictFrame(frameIndex);
  loadPageIntoFrame(processId, pageNumber, frameIndex);
  m_stats.totalReplacements++;

  return PageRequestResult::REPLACEMENT;
}

void OptimalMemoryManager::allocateForProcess(int processId, int requiredPages) {
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

void OptimalMemoryManager::freeForProcess(int processId) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_pageTables.find(processId);
  if (it == m_pageTables.end()) return;

  for (Frame& frame : m_frames) {
    if (frame.pid == processId) {
      frame.reset();
      m_stats.usedFrames--;
    }
  }

  m_futureRefs.erase(processId);
  m_pageTables.erase(it);
}

void OptimalMemoryManager::completePageLoad(int processId, int pageNumber) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_pageTables.find(processId);
  if (it == m_pageTables.end()) return;

  auto pageIt = it->second.find(pageNumber);
  if (pageIt != it->second.end() && pageIt->second.present) {
    pageIt->second.lastAccess = *m_clockRef;
  }
}

void OptimalMemoryManager::registerFutureReferences(int processId,
                                                    const std::vector<int>& referenceString) {
  std::lock_guard<std::mutex> lock(m_mutex);

  ProcessFutureReferences refs;
  refs.processId = processId;
  refs.futurePages = referenceString;
  refs.currentIndex = 0;

  m_futureRefs[processId] = refs;
}

void OptimalMemoryManager::advanceInstructionPointer(int processId) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_futureRefs.find(processId);
  if (it != m_futureRefs.end()) {
    if (it->second.currentIndex < it->second.futurePages.size()) {
      it->second.currentIndex++;
    }
  }
}

std::vector<waos::common::FrameInfo> OptimalMemoryManager::getFrameStatus() const {
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

std::vector<waos::common::PageTableEntryInfo> OptimalMemoryManager::getPageTableForProcess(int processId) const {
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

waos::common::MemoryStats OptimalMemoryManager::getMemoryStats() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  waos::common::MemoryStats currentStats = m_stats;
  uint64_t totalAccesses = m_stats.totalPageFaults + m_totalHits;
  currentStats.hitRatio = (totalAccesses > 0) ? (double)m_totalHits / totalAccesses * 100.0 : 0.0;
  return currentStats;
}

std::string OptimalMemoryManager::getAlgorithmName() const {
  return "Optimal (Theoretical)";
}

void OptimalMemoryManager::reset() {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Clear frames
  for (auto& frame : m_frames) {
    frame.reset();
  }

  // Clear page tables
  m_pageTables.clear();

  // Clear future references
  m_futureRefs.clear();

  // Reset stats
  m_stats.usedFrames = 0;
  m_stats.totalPageFaults = 0;
  m_stats.totalReplacements = 0;
  m_stats.hitRatio = 0.0;
  m_stats.faultsPerProcess.clear();
  m_totalHits = 0;
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
  m_stats.usedFrames++;
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
  m_stats.usedFrames--;
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

}  // namespace waos::memory
