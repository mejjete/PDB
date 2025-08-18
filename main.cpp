#include <QApplication>
#include <PDB.hpp>
#include "mainwindow.h"
#include <memory>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    pdb::PDBDebug pdb_instance;
    pdb_instance.launch("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3",
                        pdb::PDB_Debug_type::GDB);

    MainWindow mainWindow;
    mainWindow.resize(720, 480);
    mainWindow.setWindowTitle("PDB UI (beta)");
    mainWindow.show();

    pdb_instance.join(0);

    return app.exec();
}
