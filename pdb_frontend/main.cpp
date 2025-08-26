#include "mainwindow.h"
#include <QApplication>
#include <memory>

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  MainWindow mainWindow;
  mainWindow.resize(720, 480);
  mainWindow.setWindowTitle("PDB UI (beta)");
  mainWindow.show();
  return app.exec();
}
