/**
 * @brief Defines the central simulation clock.
 * @version 0.1
 * @date 11-11-2025
 */

#pragma once

#include <cstdint>

namespace waos::core {

/**
 * @class Clock
 * @brief Manages the discrete simulation time.
 *
 * This class provides a centralized mechanism for advancing and querying
 * the global simulation time, measured in "ticks".
 */
class Clock {
 public:
  /**
   * @brief Constructs a Clock, initialized to time 0.
   */
  Clock();

  /**
   * @brief Advances the simulation time by one tick.
   */
  void tick();

  /**
   * @brief Resets the simulation time to 0.
   */
  void reset();

  /**
   * @brief Gets the current simulation time.
   * @return The current number of ticks since the simulation started.
   */
  uint64_t getTime() const;

  /**
   * @brief Gets a pointer to the current simulation time.
   * Used by components that need a reference to the clock (e.g. MemoryManager).
   * @return Pointer to the internal time counter.
   */
  const uint64_t* getTimeAddress() const;

 private:
  uint64_t m_currentTime;
};

}  // namespace waos::core
