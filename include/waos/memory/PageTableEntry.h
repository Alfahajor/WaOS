#pragma once

#include <cstdint>

namespace waos::memory {

  /**
   * @struct PageTableEntry
   * @brief Represents an entry in a process's page table
   *
   * This structure maps a logical page number to a physical frame number
   * and contains control bits for memory management
   */
  struct PageTableEntry {
    int frameNumber = -1;       // Physical frame number (-1 if not loaded)
    bool present = false;       // Present bit: true if page is in physical memory
    uint64_t lastAccess = 0;    // Timestamp of last access (for LRU algorithm)
    bool referenced = false;    // Referenced bit (for some algorithms)
    bool modified = false;      // Modified/Dirty bit (for some algorithms)

    /**
     * @brief Checks if the page is currently loaded in memory
     */
    bool isLoaded() const {
      return present;
    }

    /**
     * @brief Marks the page as loaded in the specified frame
     */
    void load(int frame, uint64_t currentTime) {
      frameNumber = frame;
      present = true;
      lastAccess = currentTime;
      referenced = true;
    }

    /**
     * @brief Marks the page as not present (evicted from memory)
     */
    void evict() {
      frameNumber = -1;
      present = false;
      referenced = false;
    }
  };

}