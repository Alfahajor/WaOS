/**
 * @brief Updated Interface with query capability.
 * @version 0.2
 */

#pragma once

#include <cstdint>

namespace waos::memory {

  enum class PageRequestResult {
    HIT,
    PAGE_FAULT,
    REPLACEMENT
  };

  /**
   * @brief Abstract base interface for memory managers
   *
   * Implementations must handle page requests, allocation, and deallocation
   * of process memory. The convention here is that handlePageRequest()
   * returns true if the page is successfully loaded (or already present),
   * and false if a page fault occurs
   */
  class IMemoryManager {
  public:
    virtual ~IMemoryManager() = default;

    /**
     * @brief Checks if a specific page is currently loaded in a frame.
     * This method is READ-ONLY and does not trigger replacement logic.
     *
     * @param processId The PID.
     * @param pageNumber The virtual page number.
     * @return true if loaded, false otherwise.
     */
    virtual bool isPageLoaded(int processId, int pageNumber) const = 0;

    /**
     * @brief Handles a formal page request. Triggers load/replacement if needed.
     * @return Result of the operation (HIT or FAULT).
     */
    virtual PageRequestResult requestPage(int processId, int pageNumber) = 0;

    /**
     * @brief Allocate memory structures for a new process.
     *
     * Need to be used before the process can make page requests
     * Creates the page table and reserves necessary structures for
     * the process based on its required pages
     */
    virtual void allocateForProcess(int processId, int requiredPages) = 0;

    /**
     * @brief Deallocate memory structures for a terminated process
     *
     * Frees all frames used by the process and removes its page table
     */
    virtual void freeForProcess(int processId) = 0;

    /**
     * @brief Marks a page as successfully loaded into a frame.
     * Should be called by the Simulator after the I/O penalty time expires.
     */
    virtual void completePageLoad(int processId, int pageNumber) = 0;
  };

}
