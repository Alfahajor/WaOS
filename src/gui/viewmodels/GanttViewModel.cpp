#include "GanttViewModel.h"

#include <QColor>

namespace waos::gui::viewmodels {

GanttViewModel::GanttViewModel(QObject* parent) : QAbstractListModel(parent) {}

void GanttViewModel::setSimulator(waos::core::Simulator* simulator) {
  m_simulator = simulator;
  if (m_simulator) {
    connect(m_simulator, &waos::core::Simulator::clockTicked,
            this, &GanttViewModel::onClockTicked);
  }
}

int GanttViewModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  return static_cast<int>(m_blocks.size());
}

QVariant GanttViewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= static_cast<int>(m_blocks.size()))
    return QVariant();

  const auto& block = m_blocks[index.row()];

  switch (role) {
    case PidRole:
      if (block.pid == -1) return "IDLE";
      if (block.pid == -2) return "CS";
      return QString::number(block.pid);
    case StartTickRole:
      return static_cast<qulonglong>(block.startTick);
    case EndTickRole:
      return static_cast<qulonglong>(block.endTick);
    case DurationRole:
      return static_cast<int>(block.endTick - block.startTick);
    case ColorRole: {
      if (block.pid == -1) return QColor("#181825");  // IDLE
      if (block.pid == -2) return QColor("#45475a");  // CS
      // Generate a consistent color based on PID
      int hue = (block.pid * 137) % 360;
      return QColor::fromHsl(hue, 200, 150);
    }
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> GanttViewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PidRole] = "pid";
  roles[StartTickRole] = "startTick";
  roles[EndTickRole] = "endTick";
  roles[DurationRole] = "duration";
  roles[ColorRole] = "color";
  return roles;
}

void GanttViewModel::onClockTicked(uint64_t tick) {
  if (!m_simulator) return;

  m_totalTicks = tick;
  emit totalTicksChanged();

  const auto* runningProcess = m_simulator->getRunningProcess();
  int runningPid = -1;  // Default to IDLE

  if (runningProcess) {
    runningPid = runningProcess->getPid();
  } else if (m_simulator->isContextSwitching()) {
    runningPid = -2;  // CS
  }

  // If process changed, close current block and start new one
  if (runningPid != m_currentPid) {
    // Close previous block if it wasn't the initial state
    if (m_currentPid != -3 || (m_blocks.empty() && tick > 0)) {  // -3 is initial uninitialized state
      // If we had a running process (or idle/cs), finish its block
      // Now we record EVERYTHING including IDLE (-1) and CS (-2)

      // Avoid adding block if duration is 0 (e.g. immediate switch)
      if (tick > m_currentBlockStart) {
        beginInsertRows(QModelIndex(), m_blocks.size(), m_blocks.size());
        m_blocks.push_back({m_currentPid, m_currentBlockStart, tick});
        endInsertRows();
      }
    }

    m_currentPid = runningPid;
    m_currentBlockStart = tick;
  }

  // Update running totals
  if (runningPid == -1) {
    m_idleTime++;
    emit idleTimeChanged();
  } else if (runningPid == -2) {
    m_contextSwitchTime++;
    emit contextSwitchTimeChanged();
  } else {
    m_effectiveTime++;
    emit effectiveTimeChanged();
  }
}

void GanttViewModel::reset() {
  beginResetModel();
  m_blocks.clear();
  endResetModel();
  m_currentPid = -3;  // Reset to uninitialized
  m_currentBlockStart = 0;
  m_totalTicks = 0;
  emit totalTicksChanged();

  m_idleTime = 0;
  emit idleTimeChanged();
  m_contextSwitchTime = 0;
  emit contextSwitchTimeChanged();
  m_effectiveTime = 0;
  emit effectiveTimeChanged();
}

}  // namespace waos::gui::viewmodels
