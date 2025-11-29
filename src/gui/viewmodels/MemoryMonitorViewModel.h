#pragma once
#include <QList>
#include <QObject>

#include "../mock/MockSimulator.h"
#include "../models/FrameItemModel.h"

namespace waos::gui::viewmodels {

class MemoryMonitorViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<QObject*> frameList READ frameList NOTIFY frameListChanged)
  Q_PROPERTY(int totalPageFaults READ totalPageFaults NOTIFY totalPageFaultsChanged)
  Q_PROPERTY(int totalReplacements READ totalReplacements NOTIFY totalReplacementsChanged)
  Q_PROPERTY(double hitRatio READ hitRatio NOTIFY hitRatioChanged)

 public:
  explicit MemoryMonitorViewModel(QObject* parent = nullptr);

  void setSimulator(waos::gui::mock::MockSimulator* simulator);
  QList<QObject*> frameList() const { return m_frameItems; }

  int totalPageFaults() const { return m_totalPageFaults; }
  int totalReplacements() const { return m_totalReplacements; }
  double hitRatio() const { return m_hitRatio; }

 public slots:
  void onClockTicked(uint64_t tick);

 signals:
  void frameListChanged();
  void totalPageFaultsChanged();
  void totalReplacementsChanged();
  void hitRatioChanged();

 private:
  waos::gui::mock::MockSimulator* m_simulator = nullptr;
  QList<QObject*> m_frameItems;

  int m_totalPageFaults = 0;
  int m_totalReplacements = 0;
  double m_hitRatio = 0.0;
};

}  // namespace waos::gui::viewmodels
