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

#include "controllers/SimulationController.h"
#include "viewmodels/MemoryMonitorViewModel.h"
#include "viewmodels/ProcessMonitorViewModel.h"

// Controllers and ViewModels will be registered here

int main(int argc, char* argv[]) {
  Q_INIT_RESOURCE(resources);

  QGuiApplication app(argc, argv);

  app.setOrganizationName("UNSA");
  app.setOrganizationDomain("unsa.edu.pe");
  app.setApplicationName("WaOS Simulator");

  QQmlApplicationEngine engine;

  // Instantiate Controller and ViewModels
  auto* controller = new waos::gui::controllers::SimulationController(&app);
  auto* processVM = new waos::gui::viewmodels::ProcessMonitorViewModel(&app);
  auto* memoryVM = new waos::gui::viewmodels::MemoryMonitorViewModel(&app);

  // Link them
  controller->registerProcessViewModel(processVM);
  controller->registerMemoryViewModel(memoryVM);

  // Register as context properties (Global objects in QML)
  engine.rootContext()->setContextProperty("simulationController", controller);
  engine.rootContext()->setContextProperty("processViewModel", processVM);
  engine.rootContext()->setContextProperty("memoryViewModel", memoryVM);

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
