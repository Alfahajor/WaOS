#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <vector>
#include "../mock/MockSimulator.h"

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

    void setSimulator(waos::gui::mock::MockSimulator* simulator);
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
    waos::gui::mock::MockSimulator* m_simulator = nullptr;
    std::vector<GanttBlock> m_blocks;
    int m_totalTicks = 0;
    
    // State tracking for current block
    int m_currentPid = -1;
    uint64_t m_currentBlockStart = 0;
};

} // namespace waos::gui::viewmodels
