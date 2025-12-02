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
        case PidRole: return block.pid;
        case StartTickRole: return static_cast<qulonglong>(block.startTick);
        case EndTickRole: return static_cast<qulonglong>(block.endTick);
        case DurationRole: return static_cast<int>(block.endTick - block.startTick);
        case ColorRole: {
            // Generate a consistent color based on PID
            int hue = (block.pid * 137) % 360;
            return QColor::fromHsl(hue, 200, 150);
        }
        default: return QVariant();
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
    int runningPid = runningProcess ? runningProcess->getPid() : -1; // -1 for Idle

    // If process changed, close current block and start new one
    if (runningPid != m_currentPid) {
        // Close previous block if it wasn't the initial state
        if (m_currentPid != -1 || (m_blocks.empty() && tick > 0)) {
            // If we had a running process (or idle), finish its block
            // Note: If m_currentPid was -1 (Idle), we can choose to record it or not.
            // Let's record Idle as PID 0 or -1 for gaps.
            
            if (m_currentPid != -1) { // Only record actual processes for now
                beginInsertRows(QModelIndex(), m_blocks.size(), m_blocks.size());
                m_blocks.push_back({m_currentPid, m_currentBlockStart, tick});
                endInsertRows();
            }
        }
        
        m_currentPid = runningPid;
        m_currentBlockStart = tick;
    }
}

void GanttViewModel::reset() {
    beginResetModel();
    m_blocks.clear();
    endResetModel();
    m_currentPid = -1;
    m_currentBlockStart = 0;
    m_totalTicks = 0;
    emit totalTicksChanged();
}

} // namespace waos::gui::viewmodels
