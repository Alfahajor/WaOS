/**
 * @brief Updated Interface with query capability.
 * @version 0.2
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "waos/common/DataStructures.h"

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

  /**
   * @brief Optional: Register future page references for optimal algorithms.
   * Only OptimalMemoryManager implements this. Other algorithms can ignore it.
   * @param processId Process identifier.
   * @param referenceString Complete sequence of future page references.
   */
  virtual void registerFutureReferences(int processId, const std::vector<int>& referenceString) {
    // Default implementation does nothing (for FIFO, LRU)
    (void)processId;
    (void)referenceString;
  }

  /**
   * @brief Optional: Advance instruction pointer for optimal algorithms.
   * Only OptimalMemoryManager uses this. Other algorithms can ignore it.
   * @param processId Process identifier.
   */
  virtual void advanceInstructionPointer(int processId) {
    // Default implementation does nothing (for FIFO, LRU)
    (void)processId;
  }

  /**
   * @brief Obtiene el estado visual de todos los frames físicos.
   * Retorna vector ordenado por Frame ID.
   */
  virtual std::vector<waos::common::FrameInfo> getFrameStatus() const = 0;

  /**
   * @brief Obtiene la tabla de páginas para visualización.
   */
  virtual std::vector<waos::common::PageTableEntryInfo> getPageTableForProcess(int processId) const = 0;

  /**
   * @brief Obtiene estadísticas acumuladas de memoria (Hits, Faults, etc).
   */
  virtual waos::common::MemoryStats getMemoryStats() const = 0;

  /**
   * @brief Obtiene el nombre del algoritmo (ej: "LRU", "FIFO").
   */
  virtual std::string getAlgorithmName() const = 0;

  /**
   * @brief Resets the memory manager state.
   * Clears all frames, page tables, and statistics.
   */
  virtual void reset() = 0;
};

}  // namespace waos::memory
