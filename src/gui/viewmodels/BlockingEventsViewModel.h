#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <set>
#include <vector>

#include "../mock/MockSimulator.h"

namespace waos::gui::viewmodels {

struct BlockedProcessInfo {
  int pid;
  QString reason;  // "I/O" or "Memory"
  int waitTime;
};

class BlockingEventsViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<QObject*> ioBlockedList READ ioBlockedList NOTIFY ioBlockedListChanged)
  Q_PROPERTY(QList<QObject*> memoryBlockedList READ memoryBlockedList NOTIFY memoryBlockedListChanged)
  Q_PROPERTY(QStringList notifications READ notifications NOTIFY notificationsChanged)

 public:
  explicit BlockingEventsViewModel(QObject* parent = nullptr);

  void setSimulator(waos::gui::mock::MockSimulator* simulator);

  QList<QObject*> ioBlockedList() const { return m_ioBlockedItems; }
  QList<QObject*> memoryBlockedList() const { return m_memoryBlockedItems; }
  QStringList notifications() const { return m_notifications; }

 public slots:
  void onClockTicked(uint64_t tick);
  void reset();

 signals:
  void ioBlockedListChanged();
  void memoryBlockedListChanged();
  void notificationsChanged();

 private:
  waos::gui::mock::MockSimulator* m_simulator = nullptr;

  QList<QObject*> m_ioBlockedItems;
  QList<QObject*> m_memoryBlockedItems;
  QStringList m_notifications;

  // State tracking for notifications
  std::set<int> m_prevIoBlockedPids;
  std::set<int> m_prevMemoryBlockedPids;
};

// Simple model for the list view items
class BlockedItemModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(int pid READ pid CONSTANT)
  Q_PROPERTY(QString reason READ reason CONSTANT)
  Q_PROPERTY(QString info READ info CONSTANT)

 public:
  BlockedItemModel(int pid, QString reason, QString info, QObject* parent = nullptr)
      : QObject(parent), m_pid(pid), m_reason(reason), m_info(info) {}

  int pid() const { return m_pid; }
  QString reason() const { return m_reason; }
  QString info() const { return m_info; }

 private:
  int m_pid;
  QString m_reason;
  QString m_info;
};

}  // namespace waos::gui::viewmodels
