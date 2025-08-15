#include <PDB.hpp>
#include <QApplication>
#include <mainwindow.h>
#include <memory>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  pdb::PDBDebug pdb_instance;
  pdb_instance.launch("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3",
                      pdb::PDB_Debug_type::GDB);

  MainWindow w;
  w.show();

  pdb_instance.join(0);
  return a.exec();
}
