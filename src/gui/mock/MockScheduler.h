#pragma once

#include <deque>
#include <string>
#include <vector>

#include "waos/common/DataStructures.h"
#include "waos/scheduler/IScheduler.h"

namespace waos::gui::mock {

class MockScheduler : public waos::scheduler::IScheduler {
 public:
  MockScheduler();
  ~MockScheduler() override = default;

  // IScheduler interface
  void addProcess(waos::core::Process* p) override;
  waos::core::Process* getNextProcess() override;
  bool hasReadyProcesses() const override;
  int getTimeSlice() const override;

  // GUI Requirements
  std::vector<const waos::core::Process*> peekReadyQueue() const;  // Changed to match IScheduler requirement if it was virtual there, but it's not in IScheduler yet (it's a new requirement).
  // Wait, the requirements said "Add to IScheduler.h". Since I cannot modify IScheduler.h (it belongs to another team), I am implementing it here as if it was there, or just as a method of MockScheduler.
  // The user said "APIs Requeridas en IScheduler".
  // If I am making a Mock that inherits IScheduler, and IScheduler DOES NOT have these methods yet (because the other team hasn't added them), I can add them to MockScheduler but I cannot override them.
  // However, the ViewModel will likely use `IScheduler*` if it was the real system.
  // But for the Stub, I am using `MockSimulator` which can return `MockScheduler*`.
  // So I will implement them as non-virtual methods of MockScheduler for now, or virtual if I assume the interface will change.
  // I'll just declare them here.

  size_t getReadyQueueSize() const;
  std::string getAlgorithmName() const;  // This IS in the requirements to be added to IScheduler.
  waos::common::SchedulerMetrics getSchedulerMetrics() const;

 private:
  std::deque<waos::core::Process*> m_readyQueue;
  int m_timeSlice = 4;
};

}  // namespace waos::gui::mock
