#include "ExecutionLogViewModel.h"

namespace waos::gui::viewmodels {

ExecutionLogViewModel::ExecutionLogViewModel(QObject* parent) : QAbstractListModel(parent) {}

void ExecutionLogViewModel::setSimulator(waos::gui::mock::MockSimulator* simulator) {
    m_simulator = simulator;
    if (m_simulator) {
        connect(m_simulator, &waos::gui::mock::MockSimulator::clockTicked, 
                this, &ExecutionLogViewModel::onClockTicked);
    }
}

int ExecutionLogViewModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_logs.size());
}

QVariant ExecutionLogViewModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_logs.size()))
        return QVariant();

    const auto& entry = m_logs[index.row()];

    if (role == MessageRole) {
        return entry.message;
    }
    return QVariant();
}

QHash<int, QByteArray> ExecutionLogViewModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[MessageRole] = "message";
    return roles;
}

void ExecutionLogViewModel::onClockTicked(uint64_t tick) {
    if (!m_simulator) return;

    auto processes = m_simulator->getAllProcesses();
    std::map<int, waos::core::ProcessState> currentStates;

    for (const auto* p : processes) {
        currentStates[p->getPid()] = p->getState();
    }

    // Check for changes
    for (const auto& [pid, state] : currentStates) {
        if (m_previousStates.find(pid) != m_previousStates.end()) {
            waos::core::ProcessState oldState = m_previousStates[pid];
            if (oldState != state) {
                QString msg;
                if (state == waos::core::ProcessState::RUNNING) {
                    msg = QString("t=%1 → PID%2 entra a CPU").arg(tick).arg(pid);
                } else if (oldState == waos::core::ProcessState::RUNNING) {
                    if (state == waos::core::ProcessState::BLOCKED) {
                        msg = QString("t=%1 → PID%2 pasa a bloqueo E/S").arg(tick).arg(pid);
                    } else if (state == waos::core::ProcessState::READY) {
                        msg = QString("t=%1 → PID%2 sale de CPU (Preemption)").arg(tick).arg(pid);
                    } else if (state == waos::core::ProcessState::TERMINATED) {
                        msg = QString("t=%1 → PID%2 termina ejecución").arg(tick).arg(pid);
                    } else {
                        msg = QString("t=%1 → PID%2 sale de CPU").arg(tick).arg(pid);
                    }
                }
                
                if (!msg.isEmpty()) {
                    beginInsertRows(QModelIndex(), m_logs.size(), m_logs.size());
                    m_logs.push_back({msg});
                    endInsertRows();
                }
            }
        } else {
            // New process?
             if (state == waos::core::ProcessState::RUNNING) {
                QString msg = QString("t=%1 → PID%2 entra a CPU (Nuevo)").arg(tick).arg(pid);
                beginInsertRows(QModelIndex(), m_logs.size(), m_logs.size());
                m_logs.push_back({msg});
                endInsertRows();
             }
        }
    }

    m_previousStates = currentStates;
}

void ExecutionLogViewModel::reset() {
    beginResetModel();
    m_logs.clear();
    m_previousStates.clear();
    endResetModel();
}

} // namespace waos::gui::viewmodels
