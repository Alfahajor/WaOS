#include "waos/core/Parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <optional>

namespace waos::core {

  std::vector<ProcessInfo> Parser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      throw std::runtime_error("Parser Error: Could not open file " + filePath);
    }

    std::vector<ProcessInfo> processes;
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
      lineNumber++;
      if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments

      auto processInfoOpt = parseLine(line, lineNumber);
      if (processInfoOpt) processes.push_back(*processInfoOpt);
    }

    return processes;
  }

  std::optional<ProcessInfo> Parser::parseLine(const std::string& line, int lineNumber) {
    std::stringstream ss(line);
    ProcessInfo info;
    std::string pidStr, burstsStr;

    ss >> pidStr >> info.arrivalTime;

    // Read all bursts hasta encontrar un token que no empiece con 'C' o 'E'
    std::string currentToken;
    while (ss >> currentToken) {
      if (currentToken.rfind("CPU(", 0) == 0 || currentToken.rfind("E/S(", 0) == 0) {
        burstsStr += currentToken;
      } else {
        // El token actual no es una ráfaga, es la prioridad
        try {
          info.priority = std::stoi(currentToken);
        } catch (...) {
          std::cerr << "Warning: Error parsing priority on line " << lineNumber << std::endl;
          return std::nullopt;
        }
        break;
      }
    }
    
    ss >> info.requiredPages;

    if (ss.fail()) {
      std::cerr << "Warning: Invalid format on line " << lineNumber << ". Skipping." << std::endl;
      return std::nullopt;
    }
    
    // Parse PID ("P1" to 1)
    try {
      if (pidStr.length() > 1 && (pidStr[0] == 'P' || pidStr[0] == 'p')) {
        info.pid = std::stoi(pidStr.substr(1));
      } else {
        info.pid = std::stoi(pidStr);
      }
    } catch (...) {
      std::cerr << "Warning: Invalid PID on line " << lineNumber << std::endl;
      return std::nullopt;
    }

    // Parse Bursts "CPU(4),E/S(3),CPU(5)"
    std::stringstream burstStream(burstsStr);
    std::string segment;

    while(std::getline(burstStream, segment, ',')) {
      if (segment.empty()) continue;

      Burst burst;
      size_t openParen = segment.find('(');
      size_t closeParen = segment.find(')');

      if (openParen != std::string::npos && closeParen != std::string::npos) {
        if (segment.rfind("CPU", 0) == 0) {
          burst.type = BurstType::CPU;
        } else if (segment.rfind("E/S", 0) == 0) {
          burst.type = BurstType::IO;
        } else {
          continue; // Unknown type
        }

        try {
          std::string valueStr = segment.substr(openParen + 1, closeParen - openParen - 1);
          burst.duration = std::stoi(valueStr);
          info.bursts.push(burst);
        } catch (...) {
          std::cerr << "Warning: Invalid burst duration '" 
            << segment << "' on line " << lineNumber << std::endl;
        }
      }
    }

    if (info.bursts.empty()) {
      std::cerr << "Warning: No valid CPU bursts found on line "
        << lineNumber << ". Skipping." << std::endl;
      return std::nullopt;
    }

    return info;
  }

}
