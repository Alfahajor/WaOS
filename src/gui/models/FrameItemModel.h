#pragma once
#include <QObject>
#include <QString>

namespace waos::gui::models {

class FrameItemModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(int frameId READ frameId WRITE setFrameId NOTIFY frameIdChanged)
  Q_PROPERTY(int pid READ pid WRITE setPid NOTIFY pidChanged)
  Q_PROPERTY(bool occupied READ occupied WRITE setOccupied NOTIFY occupiedChanged)
  Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
  Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)

 public:
  explicit FrameItemModel(QObject* parent = nullptr) : QObject(parent) {}

  int frameId() const { return m_frameId; }
  void setFrameId(int id) {
    if (m_frameId != id) {
      m_frameId = id;
      emit frameIdChanged();
    }
  }

  int pid() const { return m_pid; }
  void setPid(int pid) {
    if (m_pid != pid) {
      m_pid = pid;
      emit pidChanged();
    }
  }

  bool occupied() const { return m_occupied; }
  void setOccupied(bool occupied) {
    if (m_occupied != occupied) {
      m_occupied = occupied;
      emit occupiedChanged();
    }
  }

  QString label() const { return m_label; }
  void setLabel(const QString& label) {
    if (m_label != label) {
      m_label = label;
      emit labelChanged();
    }
  }

  QString color() const { return m_color; }
  void setColor(const QString& color) {
    if (m_color != color) {
      m_color = color;
      emit colorChanged();
    }
  }

 signals:
  void frameIdChanged();
  void pidChanged();
  void occupiedChanged();
  void labelChanged();
  void colorChanged();

 private:
  int m_frameId = 0;
  int m_pid = -1;
  bool m_occupied = false;
  QString m_label;
  QString m_color;
};

}  // namespace waos::gui::models
