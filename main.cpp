#include "mainwindow.h"
#include <PDB.hpp>
#include <QApplication>
#include <memory>

using Debugger = pdb::PDBDebug<pdb::GDBDebugger>;

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  using namespace pdb;
  auto debug =
      Debugger::create("mpirun -np 4", "/usr/bin/gdb", "./mpi_test.out");
  MainWindow mainWindow;
  mainWindow.resize(720, 480);
  mainWindow.setWindowTitle("PDB UI (beta)");
  mainWindow.show();
  return app.exec();
}
