#include <QApplication>
#include <QWidget>

#include "mainwindow.h"

int makeWindow(int argc, char* argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
