/**
 * @brief Defines a utility for parsing process definition files.
 * @version 0.1
 * @date 10-20-2025
 */

#pragma once

#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <optional>

namespace waos::core {

  /**
   * @struct ProcessInfo
   * @brief A Data Transfer Object (DTO) to hold parsed process data.
   *
   * This struct is used as an intermediate representation of a process,
   * decoupling the Parser from the concrete Process class.
   */
  struct ProcessInfo {
    int pid;
    uint64_t arrivalTime;
    int priority;
    int requiredPages;
    std::queue<int> cpuBursts;
  };

}
