#include "MemoryMonitorViewModel.h"

namespace waos::gui::viewmodels {

MemoryMonitorViewModel::MemoryMonitorViewModel(QObject* parent) : QObject(parent) {}

void MemoryMonitorViewModel::setSimulator(waos::gui::mock::MockSimulator* simulator) {
  m_simulator = simulator;
  if (m_simulator) {
    connect(m_simulator, &waos::gui::mock::MockSimulator::clockTicked,
            this, &MemoryMonitorViewModel::onClockTicked);
  }
}

void MemoryMonitorViewModel::onClockTicked(uint64_t tick) {
  if (!m_simulator) return;

  auto* memoryManager = m_simulator->getMemoryManager();
  if (!memoryManager) return;

  auto frames = memoryManager->getFrameStatus();

  qDeleteAll(m_frameItems);
  m_frameItems.clear();

  for (const auto& frameInfo : frames) {
    auto* item = new waos::gui::models::FrameItemModel(this);

    item->setFrameId(frameInfo.frameId);
    item->setOccupied(frameInfo.isOccupied);

    if (frameInfo.isOccupied) {
      QString label = QString("P%1:%2")
                          .arg(frameInfo.ownerPid)
                          .arg(frameInfo.pageNumber);
      item->setLabel(label);
      item->setColor("#4CAF50");  // Green
    } else {
      item->setLabel("Libre");
      item->setColor("#9E9E9E");  // Grey
    }

    m_frameItems.append(item);
  }

  emit frameListChanged();

  auto stats = memoryManager->getMemoryStats();
  if (m_totalPageFaults != stats.totalPageFaults) {
    m_totalPageFaults = stats.totalPageFaults;
    emit totalPageFaultsChanged();
  }
  if (m_totalReplacements != stats.totalReplacements) {
    m_totalReplacements = stats.totalReplacements;
    emit totalReplacementsChanged();
  }
  if (m_hitRatio != stats.hitRatio) {
    m_hitRatio = stats.hitRatio;
    emit hitRatioChanged();
  }
}

}  // namespace waos::gui::viewmodels
