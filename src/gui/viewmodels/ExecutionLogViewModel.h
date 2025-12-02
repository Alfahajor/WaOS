#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <map>
#include "../mock/MockSimulator.h"

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

    void setSimulator(waos::gui::mock::MockSimulator* simulator);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void onClockTicked(uint64_t tick);
    void reset();

private:
    waos::gui::mock::MockSimulator* m_simulator = nullptr;
    std::vector<LogEntry> m_logs;
    
    // State tracking
    std::map<int, waos::core::ProcessState> m_previousStates;
};

} // namespace waos::gui::viewmodels
