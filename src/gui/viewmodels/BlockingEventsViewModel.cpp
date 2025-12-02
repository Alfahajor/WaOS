#include "BlockingEventsViewModel.h"

namespace waos::gui::viewmodels {

BlockingEventsViewModel::BlockingEventsViewModel(QObject* parent) : QObject(parent) {}

void BlockingEventsViewModel::setSimulator(waos::core::Simulator* simulator) {
  m_simulator = simulator;
  if (m_simulator) {
    connect(m_simulator, &waos::core::Simulator::clockTicked,
            this, &BlockingEventsViewModel::onClockTicked);
  }
}

void BlockingEventsViewModel::onClockTicked(uint64_t tick) {
  if (!m_simulator) return;

  // 1. Handle I/O Blocked Processes
  auto ioBlocked = m_simulator->getBlockedProcesses();
  std::set<int> currentIoPids;

  qDeleteAll(m_ioBlockedItems);
  m_ioBlockedItems.clear();

  for (const auto* p : ioBlocked) {
    currentIoPids.insert(p->getPid());
    // In a real scenario, we'd get the specific I/O device or remaining time
    m_ioBlockedItems.append(new BlockedItemModel(p->getPid(), "I/O", "Esperando E/S", this));
  }
  emit ioBlockedListChanged();

  // Check for unblocked I/O processes
  for (int pid : m_prevIoBlockedPids) {
    if (currentIoPids.find(pid) == currentIoPids.end()) {
      // Process is no longer blocked. Check if it's terminated or ready.
      // For simplicity, we assume if it left blocked, it's a "Ready" transition or similar.
      m_notifications.prepend(QString("t=%1: Proceso %2 desbloqueado de E/S -> Listo").arg(tick).arg(pid));
      if (m_notifications.size() > 50) m_notifications.removeLast();
      emit notificationsChanged();
    }
  }
  m_prevIoBlockedPids = currentIoPids;

  // 2. Handle Memory Blocked Processes
  auto memoryBlocked = m_simulator->getMemoryWaitQueue();
  std::set<int> currentMemPids;

  qDeleteAll(m_memoryBlockedItems);
  m_memoryBlockedItems.clear();

  for (const auto& info : memoryBlocked) {
    currentMemPids.insert(info.pid);
    QString details = QString("Page Fault: Page %1 (Remaining: %2 ticks)")
                          .arg(info.pageNumber)
                          .arg(info.ticksRemaining);
    m_memoryBlockedItems.append(new BlockedItemModel(info.pid, "Memoria", details, this));
  }
  emit memoryBlockedListChanged();

  // Check for unblocked Memory processes
  for (int pid : m_prevMemoryBlockedPids) {
    if (currentMemPids.find(pid) == currentMemPids.end()) {
      m_notifications.prepend(QString("t=%1: Proceso %2 obtuvo memoria -> Listo").arg(tick).arg(pid));
      if (m_notifications.size() > 50) m_notifications.removeLast();
      emit notificationsChanged();
    }
  }
  m_prevMemoryBlockedPids = currentMemPids;
}

void BlockingEventsViewModel::reset() {
  qDeleteAll(m_ioBlockedItems);
  m_ioBlockedItems.clear();
  emit ioBlockedListChanged();

  qDeleteAll(m_memoryBlockedItems);
  m_memoryBlockedItems.clear();
  emit memoryBlockedListChanged();

  m_notifications.clear();
  emit notificationsChanged();

  m_prevIoBlockedPids.clear();
  m_prevMemoryBlockedPids.clear();
}

}  // namespace waos::gui::viewmodels
