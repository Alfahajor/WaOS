#pragma once

#include <cstdint>

namespace waos::memory {

  /**
   * @struct Frame
   * @brief Represents a frame (block) of physical memory
   *
   * This structure is a pure data container that holds information about
   * a single frame in the simulated physical memory
   */
  struct Frame {
    int pid = -1;                 // Process ID that owns this frame (-1 if free)
    int pageNumber = -1;          // Page number mapped to this frame (-1 if free)
    bool occupied = false;        // True if the frame is currently in use
    uint64_t loadTime = 0;        // Timestamp when the page was loaded (for FIFO)
    uint64_t lastAccessTime = 0;  // Timestamp of last access (for LRU)

    /**
     * @brief Checks if the frame is currently free
     */
    bool isFree() const {
      return !occupied;
    }

    /**
     * @brief Resets the frame to its initial free state
     */
    void reset() {
      pid = -1;
      pageNumber = -1;
      occupied = false;
      loadTime = 0;
      lastAccessTime = 0;
    }
  };

}