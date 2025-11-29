#include "SimulationController.h"

namespace waos::gui::controllers {

SimulationController::SimulationController(QObject* parent) 
    : QObject(parent)
    , m_simulator(std::make_unique<waos::gui::mock::MockSimulator>())
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &SimulationController::onTimeout);
}

void SimulationController::start() {
    m_simulator->start();
    m_timer->start(m_tickInterval);
    emit isRunningChanged();
}

void SimulationController::stop() {
    m_simulator->stop();
    m_timer->stop();
    emit isRunningChanged();
}

void SimulationController::reset() {
    stop();
    m_simulator->reset();
}

void SimulationController::step() {
    m_simulator->tick();
}

bool SimulationController::isRunning() const {
    return m_timer->isActive();
}

int SimulationController::tickInterval() const {
    return m_tickInterval;
}

void SimulationController::setTickInterval(int interval) {
    if (m_tickInterval != interval) {
        m_tickInterval = interval;
        m_timer->setInterval(m_tickInterval);
        emit tickIntervalChanged();
    }
}

void SimulationController::registerProcessViewModel(waos::gui::viewmodels::ProcessMonitorViewModel* vm) {
    if (vm) {
        vm->setSimulator(m_simulator.get());
    }
}

void SimulationController::registerMemoryViewModel(waos::gui::viewmodels::MemoryMonitorViewModel* vm) {
    if (vm) {
        vm->setSimulator(m_simulator.get());
    }
}

void SimulationController::onTimeout() {
    m_simulator->tick();
}

} // namespace waos::gui::controllers
