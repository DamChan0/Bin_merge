#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtWidgets/QMainWindow>
#include <thread>

#include "../BinMerge/BinMerge.h"
#include "../BinMerge/fileControl.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
  template <typename... Args>
  void QprintErr(const QString& message, Args&&... args);
  template <typename... Args>
  void QprintDone(const QString& message, Args&&... args);
  template <typename... Args>
  void QprintDebug(const QString& message, const Args&... args);

 public slots:

  int launch();

 private:
  Ui::MainWindow* ui;
};
int makeWindow(int argc, char* argv[]);
#endif  // MAINWINDOW_H
