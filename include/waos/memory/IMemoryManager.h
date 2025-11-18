#pragma once

#include <cstdint>
#include <vector>

namespace waos::core {
    class Process; // forward declaration
}

namespace waos::memory {

  struct Frame; // forward declaration

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
     * @brief Handle a page request from a process.
     *
     * This is the main method called by the simulator when a process
     * needs to access a page. If the page is not in memory, a page
     * fault occurs and the algorithm must handle replacement if needed
     *
     * @param p Pointer to the Process making the request
     * @param pageNumber The logical page number being requested
     * @return true if page is in memory (or successfully loaded), false otherwise
     */
    virtual bool handlePageRequest(waos::core::Process* p, int pageNumber) = 0;

    /**
     * @brief Allocate memory structures for a new process.
     *
     * Need to be used before the process can make page requests
     * Creates the page table and reserves necessary structures for
     * the process based on its required pages
     *
     * @param p Pointer to the Process to allocate
     */
    virtual void allocateProcess(waos::core::Process* p) = 0;

    /**
     * @brief Deallocate memory structures for a terminated process
     *
     * Frees all frames used by the process and removes its page table
     *
     * @param pid The process ID to deallocate
     */
    virtual void deallocateProcess(int pid) = 0;

    /**
     * @brief Get the total number of page faults since simulation start
     */
    virtual uint64_t getPageFaults() const = 0;

    /**
     * @brief Get the total number of page replacements since simulation start
     */
    virtual uint64_t getPageReplacements() const = 0;

    /**
     * @brief Get the number of free frames currently available
     */
    virtual int getFreeFrames() const = 0;
  };

}