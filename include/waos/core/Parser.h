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

  /**
   * @class Parser
   * @brief A static utility class for parsing process files.
   *
   * This class provides functionality to read a text file formatted with
   * process definitions and transform it into a collection of ProcessInfo
   * objects.
   */
  class Parser {
  public:
    /**
     * @brief Parses a process definition file.
     *
     * Reads the specified file line by line, parsing each valid line into
     * a ProcessInfo struct. Invalid lines are skipped, and a warning is
     * printed to stderr.
     *
     * @param filePath The path to the process definition file.
     * @return A vector of ProcessInfo structs, one for each valid process.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static std::vector<ProcessInfo> parseFile(const std::string& filePath);

  private:
    /**
     * @brief Parses a single line from the process file.
     * @param line The string line to parse.
     * @param lineNumber The line number in the file, for error reporting.
     * @return An std::optional containing ProcessInfo if parsing is successful,
     *         otherwise an empty optional.
     */
    static std::optional<ProcessInfo> parseLine(const std::string& line, int lineNumber);
  };

}
