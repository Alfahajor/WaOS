#include "MemoryMonitorViewModel.h"

namespace waos::gui::viewmodels {

MemoryMonitorViewModel::MemoryMonitorViewModel(QObject* parent) : QObject(parent) {}

void MemoryMonitorViewModel::setSimulator(waos::core::Simulator* simulator) {
  m_simulator = simulator;
  if (m_simulator) {
    connect(m_simulator, &waos::core::Simulator::clockTicked,
            this, &MemoryMonitorViewModel::onClockTicked);
  }
}

void MemoryMonitorViewModel::setSelectedPid(int pid) {
  if (m_selectedPid != pid) {
    m_selectedPid = pid;
    emit selectedPidChanged();
    updatePageTable();
  }
}

void MemoryMonitorViewModel::onClockTicked(uint64_t tick) {
  if (!m_simulator) return;

  // Update Process List
  auto processes = m_simulator->getAllProcesses();
  QList<int> newPids;
  for (const auto* p : processes) {
    newPids.append(p->getPid());
  }

  if (m_processList != newPids) {
    m_processList = newPids;
    emit processListChanged();

    // Auto-select first if none selected or selected is gone
    if (!m_processList.isEmpty()) {
      if (m_selectedPid == -1 || !m_processList.contains(m_selectedPid)) {
        setSelectedPid(m_processList.first());
      }
    }
  }

  // Use Simulator wrappers to ensure correct lock order (Sim -> Mem)
  auto frames = m_simulator->getFrameStatus();

  qDeleteAll(m_frameItems);
  m_frameItems.clear();

  for (const auto& frameInfo : frames) {
    auto* item = new waos::gui::models::FrameItemModel(this);

    item->setFrameId(frameInfo.frameId);
    item->setOccupied(frameInfo.isOccupied);
    item->setPid(frameInfo.isOccupied ? frameInfo.ownerPid : -1);

    if (frameInfo.isOccupied) {
      QString label = QString("P%1:%2")
                          .arg(frameInfo.ownerPid)
                          .arg(frameInfo.pageNumber);
      item->setLabel(label);
      item->setColor("#4CAF50");  // Green
    } else {
      item->setLabel("Free");
      item->setColor("#9E9E9E");  // Grey
    }

    m_frameItems.append(item);
  }

  emit frameListChanged();

  auto stats = m_simulator->getMemoryStats();
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

  updatePageTable();
}

void MemoryMonitorViewModel::updatePageTable() {
  if (!m_simulator || m_selectedPid == -1) {
    qDeleteAll(m_pageTableItems);
    m_pageTableItems.clear();
    emit pageTableChanged();
    return;
  }

  auto pageTable = m_simulator->getPageTableForProcess(m_selectedPid);

  qDeleteAll(m_pageTableItems);
  m_pageTableItems.clear();

  for (const auto& entry : pageTable) {
    auto* item = new waos::gui::models::PageTableItemModel(
        entry.pageNumber,
        entry.frameNumber,
        entry.present,
        this);
    m_pageTableItems.append(item);
  }
  emit pageTableChanged();
}

}  // namespace waos::gui::viewmodels
