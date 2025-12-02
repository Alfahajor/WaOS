#pragma once

#include <QObject>
#include <QString>

namespace waos::gui::models {

class PageTableItemModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(int pageNumber READ pageNumber CONSTANT)
  Q_PROPERTY(QString frameNumber READ frameNumber CONSTANT)
  Q_PROPERTY(QString state READ state CONSTANT)

 public:
  explicit PageTableItemModel(int page, int frame, bool valid, QObject* parent = nullptr)
      : QObject(parent), m_page(page), m_frame(frame), m_valid(valid) {}

  int pageNumber() const { return m_page; }

  QString frameNumber() const {
    if (!m_valid || m_frame == -1) return "—";
    return QString("F%1").arg(m_frame);
  }

  QString state() const {
    return m_valid ? "Valida" : "No cargada";
  }

 private:
  int m_page;
  int m_frame;
  bool m_valid;
};

}  // namespace waos::gui::models
