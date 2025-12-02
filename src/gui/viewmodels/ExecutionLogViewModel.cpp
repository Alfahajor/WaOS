#include "ExecutionLogViewModel.h"

namespace waos::gui::viewmodels {

ExecutionLogViewModel::ExecutionLogViewModel(QObject* parent) : QAbstractListModel(parent) {}

void ExecutionLogViewModel::setSimulator(waos::core::Simulator* simulator) {
  m_simulator = simulator;
  if (m_simulator) {
    connect(m_simulator, &waos::core::Simulator::logMessage,
            this, &ExecutionLogViewModel::onLogMessage);
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

void ExecutionLogViewModel::onLogMessage(QString message) {
  beginInsertRows(QModelIndex(), m_logs.size(), m_logs.size());
  m_logs.push_back({message});
  endInsertRows();
}

void ExecutionLogViewModel::reset() {
  beginResetModel();
  m_logs.clear();
  endResetModel();
}

}  // namespace waos::gui::viewmodels
