#pragma once

#include "IMemoryManager.h"
#include "Frame.h"
#include "PageTable.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <cstdint>
#include <mutex> 

namespace waos::memory {

  /**
   * @class FIFOMemoryManager
   * @brief Implements FIFO page replacement algorithm.
   *
   * This algorithm replaces the page that has been in memory the longest.
   * It maintains a queue of loaded pages and replaces the oldest one when
   * a page fault occurs and there are no free frames.
   */
  class FIFOMemoryManager : public IMemoryManager {
  public:
    /**
     * @brief Constructs a FIFO Memory Manager.
     * @param totalFrames Total number of physical memory frames available.
     * @param clockRef Pointer to the simulation clock for timestamps.
     */
    explicit FIFOMemoryManager(int totalFrames, const uint64_t* clockRef);

    ~FIFOMemoryManager() override = default;

    // IMemoryManager interface implementation
    bool isPageLoaded(int processId, int pageNumber) const override;
    PageRequestResult requestPage(int processId, int pageNumber) override;
    void allocateForProcess(int processId, int requiredPages) override;
    void freeForProcess(int processId) override;
    void completePageLoad(int processId, int pageNumber) override;

    // Métodos de Observación para GUI
    std::vector<waos::common::FrameInfo> getFrameStatus() const override;
    std::vector<waos::common::PageTableEntryInfo> getPageTableForProcess(int processId) const override;
    waos::common::MemoryStats getMemoryStats() const override;
    std::string getAlgorithmName() const override;

  private:
    mutable std::mutex m_mutex;

    // Physical memory simulation
    std::vector<Frame> m_frames;
    const uint64_t* m_clockRef;            // Pointer to simulation clock
    
    // Per-process page tables
    std::unordered_map<int, PageTable> m_pageTables;
    
    // FIFO-specific: Queue to track load order
    std::queue<std::pair<int, int>> m_loadQueue;         // <processId, pageNumber>
    
    // Statistics
    waos::common::MemoryStats m_stats;
    uint64_t m_totalHits = 0;  // Counter for page hits

    /**
     * @brief Finds a free frame in physical memory.
     * @return Frame index if found, -1 otherwise.
     */
    int findFreeFrame() const;

    /**
     * @brief Loads a page into a specific frame.
     * @param processId Process owner.
     * @param pageNumber Virtual page number.
     * @param frameIndex Physical frame to use.
     */
    void loadPageIntoFrame(int processId, int pageNumber, int frameIndex);

    /**
     * @brief Selects a victim frame using FIFO policy.
     * @return Frame index to be replaced.
     */
    int selectVictimFrame();

    /**
     * @brief Evicts a page from a frame.
     * @param frameIndex Frame to evict.
     */
    void evictFrame(int frameIndex);
  };

}
