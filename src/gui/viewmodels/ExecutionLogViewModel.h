#pragma once

#include <QAbstractListModel>
#include <QString>
#include <map>
#include <vector>

#include "waos/core/Simulator.h"

namespace waos::gui::viewmodels {

struct LogEntry {
  QString message;
};

class ExecutionLogViewModel : public QAbstractListModel {
  Q_OBJECT
 public:
  enum Roles {
    MessageRole = Qt::UserRole + 1
  };

  explicit ExecutionLogViewModel(QObject* parent = nullptr);

  void setSimulator(waos::core::Simulator* simulator);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

 public slots:
  void onLogMessage(QString message);
  void reset();

 private:
  waos::core::Simulator* m_simulator = nullptr;
  std::vector<LogEntry> m_logs;
};

}  // namespace waos::gui::viewmodels
