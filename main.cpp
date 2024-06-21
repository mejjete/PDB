#include "mainwindow.h"
#include "pdb_manager/PDB.hpp"
#include <QApplication>
#include <memory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    pdb::PDBDebug pdb_instance("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3", 
        pdb::PDB_Debug_type::GDB);

    MainWindow w;
    w.show();

    return a.exec();
}
