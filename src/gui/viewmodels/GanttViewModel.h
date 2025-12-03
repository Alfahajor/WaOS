#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <vector>

#include "waos/core/Simulator.h"

namespace waos::gui::viewmodels {

struct GanttBlock {
  int pid;
  uint64_t startTick;
  uint64_t endTick;
};

class GanttViewModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int totalTicks READ totalTicks NOTIFY totalTicksChanged)

 public:
  enum GanttRoles {
    PidRole = Qt::UserRole + 1,
    StartTickRole,
    EndTickRole,
    DurationRole,
    ColorRole
  };

  explicit GanttViewModel(QObject* parent = nullptr);

  void setSimulator(waos::core::Simulator* simulator);
  int totalTicks() const { return m_totalTicks; }

  // QAbstractListModel interface
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

 public slots:
  void onClockTicked(uint64_t tick);
  void reset();

 signals:
  void totalTicksChanged();

 private:
  waos::core::Simulator* m_simulator = nullptr;
  std::vector<GanttBlock> m_blocks;
  int m_totalTicks = 0;

  // State tracking for current block
  int m_currentPid = -3; // -3: Uninitialized, -2: CS, -1: Idle, >=0: PID
  uint64_t m_currentBlockStart = 0;
};

}  // namespace waos::gui::viewmodels
