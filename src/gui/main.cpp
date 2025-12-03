/**
 * @file main.cpp
 * @brief Punto de entrada de la aplicación GUI WaOS Simulator
 * @version 1.0
 * @date 24-11-2025
 */

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "controllers/SimulationController.h"
#include "viewmodels/BlockingEventsViewModel.h"
#include "viewmodels/ExecutionLogViewModel.h"
#include "viewmodels/GanttViewModel.h"
#include "viewmodels/MemoryMonitorViewModel.h"
#include "viewmodels/ProcessMonitorViewModel.h"

// Controllers and ViewModels will be registered here

int main(int argc, char* argv[]) {
  Q_INIT_RESOURCE(resources);

  QQuickStyle::setStyle("Basic");
  QGuiApplication app(argc, argv);

  app.setOrganizationName("UNSA");
  app.setOrganizationDomain("unsa.edu.pe");
  app.setApplicationName("WaOS Simulator");

  QQmlApplicationEngine engine;

  // Instantiate Controller and ViewModels
  auto* controller = new waos::gui::controllers::SimulationController(&app);
  auto* processVM = new waos::gui::viewmodels::ProcessMonitorViewModel(&app);
  auto* memoryVM = new waos::gui::viewmodels::MemoryMonitorViewModel(&app);
  auto* logVM = new waos::gui::viewmodels::ExecutionLogViewModel(&app);
  auto* blockingVM = new waos::gui::viewmodels::BlockingEventsViewModel(&app);
  auto* ganttVM = new waos::gui::viewmodels::GanttViewModel(&app);

  // Link them
  controller->registerProcessViewModel(processVM);
  controller->registerMemoryViewModel(memoryVM);
  controller->registerExecutionLogViewModel(logVM);
  controller->registerBlockingEventsViewModel(blockingVM);
  controller->registerGanttViewModel(ganttVM);

  // Register as context properties (Global objects in QML)
  engine.rootContext()->setContextProperty("simulationController", controller);
  engine.rootContext()->setContextProperty("processViewModel", processVM);
  engine.rootContext()->setContextProperty("memoryViewModel", memoryVM);
  engine.rootContext()->setContextProperty("executionLogViewModel", logVM);
  engine.rootContext()->setContextProperty("blockingViewModel", blockingVM);
  engine.rootContext()->setContextProperty("ganttViewModel", ganttVM);

  // Load main QML
  const QUrl url(QStringLiteral("qrc:/main.qml"));

  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.load(url);

  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
