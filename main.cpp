#include "mainwindow.h"
#include "pdb_manager/PDB.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    PDBDebug pdb_instance("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3");

    MainWindow w;
    w.show();

    return a.exec();
}
