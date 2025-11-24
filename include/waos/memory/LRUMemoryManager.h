#pragma once

#include "IMemoryManager.h"
#include "Frame.h"
#include "PageTable.h"
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace waos::memory {

  /**
   * @class LRUMemoryManager
   * @brief Implements LRU page replacement algorithm.
   *
   * This algorithm replaces the page that has not been used for the longest
   * period of time. It tracks the last access time for each page and selects
   * the one with the oldest timestamp when replacement is needed.
   */
  class LRUMemoryManager : public IMemoryManager {
  public:
    /**
     * @brief Constructs an LRU Memory Manager.
     * @param totalFrames Total number of physical memory frames available.
     * @param clockRef Pointer to the simulation clock for timestamps.
     */
    explicit LRUMemoryManager(int totalFrames, const uint64_t* clockRef);

    ~LRUMemoryManager() override = default;

    // IMemoryManager interface implementation
    bool isPageLoaded(int processId, int pageNumber) const override;
    PageRequestResult requestPage(int processId, int pageNumber) override;
    void allocateForProcess(int processId, int requiredPages) override;
    void freeForProcess(int processId) override;
    void completePageLoad(int processId, int pageNumber) override;

    // Statistics getters
    uint64_t getPageFaults() const { return m_pageFaults; }
    uint64_t getPageReplacements() const { return m_pageReplacements; }
    int getFreeFrames() const;
    int getActivePages(int processId) const;

  private:
    // Physical memory simulation
    std::vector<Frame> m_frames;          // Array of physical frames
    const uint64_t* m_clockRef;            // Pointer to simulation clock
    
    // Per-process page tables
    std::unordered_map<int, PageTable> m_pageTables;
    
    // Statistics
    uint64_t m_pageFaults;
    uint64_t m_pageReplacements;

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
     * @brief Selects a victim frame using LRU policy.
     * @return Frame index to be replaced.
     */
    int selectVictimFrame();

    /**
     * @brief Evicts a page from a frame.
     * @param frameIndex Frame to evict.
     */
    void evictFrame(int frameIndex);

    /**
     * @brief Updates the last access time for a page.
     * @param processId Process owner.
     * @param pageNumber Virtual page number.
     */
    void updateAccessTime(int processId, int pageNumber);
  };

}
