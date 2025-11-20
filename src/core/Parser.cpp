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
      if (line.empty() || line[0] == '#') { // Skip empty lines and comments
        continue;
      }

      auto processInfoOpt = parseLine(line, lineNumber);
      if (processInfoOpt) {
        processes.push_back(*processInfoOpt);
      }
    }

    return processes;
  }

  std::optional<ProcessInfo> Parser::parseLine(const std::string& line, int lineNumber) {
    std::stringstream ss(line);
    ProcessInfo info;
    std::string pidStr, burstsStr;

    ss >> pidStr >> info.arrivalTime;

    // Leer todas las ráfagas hasta encontrar un token que no empiece con 'C' o 'E'
    std::string currentToken;
    while (ss >> currentToken) {
      if (currentToken.rfind("CPU(", 0) == 0 || currentToken.rfind("E/S(", 0) == 0) {
        burstsStr += currentToken;
      } else {
        // El token actual no es una ráfaga, es la prioridad
        std::stringstream priority_ss(currentToken);
        priority_ss >> info.priority;
        break;
      }
    }
    
    ss >> info.requiredPages;

    if (ss.fail()) {
      std::cerr << "Warning: Invalid format on line " << lineNumber << ". Skipping." << std::endl;
      return std::nullopt;
    }
    
    // Parse PID ("P1" to 1)
    if (pidStr.length() > 1 && (pidStr[0] == 'P' || pidStr[0] == 'p')) {
      info.pid = std::stoi(pidStr.substr(1));
    } else {
      info.pid = std::stoi(pidStr);
    }

    // Parse Bursts "CPU(4),E/S(3),CPU(5)"
    std::stringstream burstStream(burstsStr);
    std::string burst;
    while(std::getline(burstStream, burst, ',')) {
      size_t openParen = burst.find('(');
      size_t closeParen = burst.find(')');
      if (openParen != std::string::npos && closeParen != std::string::npos) {
        std::string valueStr = burst.substr(openParen + 1, closeParen - openParen - 1);
        info.cpuBursts.push(std::stoi(valueStr));
      }
    }

    if (info.cpuBursts.empty()) {
      std::cerr << "Warning: No valid CPU bursts found on line " << lineNumber << ". Skipping." << std::endl;
      return std::nullopt;
    }

    return info;
  }

}
