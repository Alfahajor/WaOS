#include "MockMemoryManager.h"

#include "MockDataProvider.h"

namespace waos::gui::mock {

MockMemoryManager::MockMemoryManager() {}

bool MockMemoryManager::isPageLoaded(int processId, int pageNumber) const {
  return true;
}

waos::memory::PageRequestResult MockMemoryManager::requestPage(int processId, int pageNumber) {
  return waos::memory::PageRequestResult::HIT;
}

void MockMemoryManager::allocateForProcess(int processId, int requiredPages) {}
void MockMemoryManager::freeForProcess(int processId) {}
void MockMemoryManager::completePageLoad(int processId, int pageNumber) {}

std::vector<waos::common::FrameInfo> MockMemoryManager::getFrameStatus() const {
  return MockDataProvider::generateFrameStatus(m_totalFrames, 0);
}

std::vector<waos::common::PageTableEntryInfo> MockMemoryManager::getPageTableForProcess(int processId) const {
  return MockDataProvider::generatePageTable(processId, 5, 0);
}

int MockMemoryManager::getTotalFrames() const {
  return m_totalFrames;
}

waos::common::MemoryStats MockMemoryManager::getMemoryStats() const {
  return MockDataProvider::generateMemoryStats(0);
}

std::string MockMemoryManager::getAlgorithmName() const {
  return "Mock Memory (LRU)";
}

void MockMemoryManager::reset() {
  // No-op for mock
}

}  // namespace waos::gui::mock
