#include "mainwindow.h"
#include "pdb_manager/PDB.hpp"
#include <QApplication>
#include <memory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    using namespace pdb;
    // PDBDebug pdb_instance;
    // pdb_instance.launch("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3", 
        // PDB_Debug_type::GDB);

    MainWindow w;
    w.show();

    // pdb_instance.join();
    return a.exec();
}
