#pragma once

#include <string>
#include <vector>

#include "waos/common/DataStructures.h"
#include "waos/memory/IMemoryManager.h"

namespace waos::gui::mock {

class MockMemoryManager : public waos::memory::IMemoryManager {
 public:
  MockMemoryManager();
  ~MockMemoryManager() override = default;

  // IMemoryManager interface
  bool isPageLoaded(int processId, int pageNumber) const override;
  waos::memory::PageRequestResult requestPage(int processId, int pageNumber) override;
  void allocateForProcess(int processId, int requiredPages) override;
  void freeForProcess(int processId) override;
  void completePageLoad(int processId, int pageNumber) override;

  // GUI Requirements
  std::vector<waos::common::FrameInfo> getFrameStatus() const;
  std::vector<waos::common::PageTableEntryInfo> getPageTableForProcess(int processId) const;
  int getTotalFrames() const;
  waos::common::MemoryStats getMemoryStats() const override;
  std::string getAlgorithmName() const override;
  void reset() override;

 private:
  int m_totalFrames = 64;
};

}  // namespace waos::gui::mock
