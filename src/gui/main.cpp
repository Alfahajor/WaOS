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

// Controllers and ViewModels will be registered here

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  app.setOrganizationName("UNSA");
  app.setOrganizationDomain("unsa.edu.pe");
  app.setApplicationName("WaOS Simulator");

  QQmlApplicationEngine engine;

  // TODO: Register custom types when implemented
  // qmlRegisterType<SimulationController>("WaOS", 1, 0, "SimulationController");
  // qmlRegisterType<ProcessMonitorViewModel>("WaOS", 1, 0, "ProcessMonitorViewModel");

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
