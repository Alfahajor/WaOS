#include "waos/core/Clock.h"

namespace waos::core {

Clock::Clock() : m_currentTime(0) {}

void Clock::tick() {
  // In a more complex simulation, this could trigger events.
  // For now, it just increments the time.
  ++m_currentTime;
}

void Clock::reset() {
  m_currentTime = 0;
}

uint64_t Clock::getTime() const {
  return m_currentTime;
}

const uint64_t* Clock::getTimeAddress() const {
  return &m_currentTime;
}

}  // namespace waos::core
