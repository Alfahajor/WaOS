#pragma once
#include <QList>
#include <QObject>

#include "../mock/MockSimulator.h"
#include "../models/FrameItemModel.h"
#include "../models/PageTableItemModel.h"

namespace waos::gui::viewmodels {

class MemoryMonitorViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<QObject*> frameList READ frameList NOTIFY frameListChanged)
  Q_PROPERTY(int totalPageFaults READ totalPageFaults NOTIFY totalPageFaultsChanged)
  Q_PROPERTY(int totalReplacements READ totalReplacements NOTIFY totalReplacementsChanged)
  Q_PROPERTY(double hitRatio READ hitRatio NOTIFY hitRatioChanged)

  // Page Table Support
  Q_PROPERTY(QList<int> processList READ processList NOTIFY processListChanged)
  Q_PROPERTY(int selectedPid READ selectedPid WRITE setSelectedPid NOTIFY selectedPidChanged)
  Q_PROPERTY(QList<QObject*> pageTable READ pageTable NOTIFY pageTableChanged)

 public:
  explicit MemoryMonitorViewModel(QObject* parent = nullptr);

  void setSimulator(waos::gui::mock::MockSimulator* simulator);
  QList<QObject*> frameList() const { return m_frameItems; }

  int totalPageFaults() const { return m_totalPageFaults; }
  int totalReplacements() const { return m_totalReplacements; }
  double hitRatio() const { return m_hitRatio; }

  QList<int> processList() const { return m_processList; }
  int selectedPid() const { return m_selectedPid; }
  void setSelectedPid(int pid);
  QList<QObject*> pageTable() const { return m_pageTableItems; }

 public slots:
  void onClockTicked(uint64_t tick);

 signals:
  void frameListChanged();
  void totalPageFaultsChanged();
  void totalReplacementsChanged();
  void hitRatioChanged();

  void processListChanged();
  void selectedPidChanged();
  void pageTableChanged();

 private:
  void updatePageTable();

  waos::gui::mock::MockSimulator* m_simulator = nullptr;
  QList<QObject*> m_frameItems;

  int m_totalPageFaults = 0;
  int m_totalReplacements = 0;
  double m_hitRatio = 0.0;

  QList<int> m_processList;
  int m_selectedPid = -1;
  QList<QObject*> m_pageTableItems;
};

}  // namespace waos::gui::viewmodels
