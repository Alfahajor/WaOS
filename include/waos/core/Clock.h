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
     * @brief Gets the current simulation time.
     * @return The current number of ticks since the simulation started.
     */
    uint64_t getTime() const;

  private:
    uint64_t m_currentTime;
  };

}
