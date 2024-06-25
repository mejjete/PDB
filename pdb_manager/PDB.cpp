#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include "PDB.hpp"

bool is_signaled = false;
void PDBcommand(pdb::PDBDebug &pdb_instance);

int main()
{
    using namespace pdb;

    PDBDebug pdb_instance;
    pdb_instance.launch("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3", 
        PDB_Debug_type::GDB);

    // PDBcommand(pdb_instance);

    pdb_instance.join();

    return 0;
}

void PDBcommand(pdb::PDBDebug &pdb_instance)
{
    std::cout << "All Source Files: \n";

    // Print out the full information about source files
    auto source_array = pdb_instance.getSourceFiles();
    for(auto &iter : source_array)
        printf("\033[92m%s\033[0m,", iter.c_str());
    std::cout << std::endl;

    std::cout << "---------------------------------------------------------------------------\n";

    std::string func = "main";
    auto function = pdb_instance.getFunction("main");
    std::cout << "Function: " << func << std::endl;
    std::cout << "\033[92mSource File: " << function.second << "\033[0m" << std::endl;
    std::cout << "\033[92mLine: " << function.first << "\033[0m" << std::endl;
}
