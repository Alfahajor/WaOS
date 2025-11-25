#pragma once

#include "IMemoryManager.h"
#include "Frame.h"
#include "PageTable.h"
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace waos::memory {

  /**
   * @struct ProcessFutureReferences
   * @brief Stores the future page reference sequence for a process.
   */
  struct ProcessFutureReferences {
    int processId;
    std::vector<int> futurePages;  // Complete sequence of future page accesses
    size_t currentIndex;            // Current position in the sequence
  };

  /**
   * @class OptimalMemoryManager
   * @brief Implements the Optimal page replacement algorithm.
   *
   * This algorithm replaces the page that will not be used for the longest
   * period of time in the future. It requires knowledge of future page
   * references, making it impossible to implement in real systems, but serves
   * as a theoretical benchmark for comparison with practical algorithms.
   *
   * Implementation Strategy:
   * - Each process provides its complete page reference sequence
   * - On replacement, scans future references to find optimal victim
   * - Page used farthest in the future (or never again) is replaced
   */
  class OptimalMemoryManager : public IMemoryManager {
  public:
    /**
     * @brief Constructs an Optimal Memory Manager.
     * @param totalFrames Total number of physical memory frames available.
     * @param clockRef Pointer to the simulation clock for timestamps.
     */
    explicit OptimalMemoryManager(int totalFrames, const uint64_t* clockRef);

    ~OptimalMemoryManager() override = default;

    // IMemoryManager interface implementation
    bool isPageLoaded(int processId, int pageNumber) const override;
    PageRequestResult requestPage(int processId, int pageNumber) override;
    void allocateForProcess(int processId, int requiredPages) override;
    void freeForProcess(int processId) override;
    void completePageLoad(int processId, int pageNumber) override;

    /**
     * @brief Registers the complete page reference sequence for a process.
     * 
     * This method should be called by the Simulator after allocateForProcess().
     * It extracts the reference string from the Process to enable optimal decisions.
     *
     * @param processId Process identifier.
     * @param referenceString Complete sequence of page accesses.
     */
    void registerFutureReferences(int processId, const std::vector<int>& referenceString) override;

    /**
     * @brief Advances the instruction pointer for a process.
     * Should be called by Simulator after each CPU tick to track current position.
     * @param processId Process identifier.
     */
    void advanceInstructionPointer(int processId) override;

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
    
    // Future references for optimal decision-making
    std::unordered_map<int, ProcessFutureReferences> m_futureRefs;
    
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
     * @brief Selects a victim frame using Optimal policy.
     * 
     * Scans all loaded pages and determines which will be used farthest
     * in the future (or never again). That page becomes the victim.
     *
     * @return Frame index to be replaced.
     */
    int selectVictimFrame();

    /**
     * @brief Evicts a page from a frame.
     * @param frameIndex Frame to evict.
     */
    void evictFrame(int frameIndex);

    /**
     * @brief Calculates when a page will be used next.
     * @param processId Process owner.
     * @param pageNumber Virtual page number.
     * @return Ticks until next use, or INT_MAX if never used again.
     */
    int getNextUseDistance(int processId, int pageNumber) const;
  };

}
