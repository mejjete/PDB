#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include "PDB.hpp"

bool is_signaled = false;

int main()
{
    using namespace pdb;

    PDBDebug pdb_instance;
    pdb_instance.launch("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3", 
        PDB_Debug_type::GDB);
    
    // auto source_array = pdb_instance.getSourceFiles();
    // for(auto &iter : source_array)
        // std::cout << iter << std::endl;

    std::string func = "main";
    auto function = pdb_instance.getFunction("main");
    std::cout << "Function: " << func << std::endl;
    std::cout << "\t Source File: " << function.second << std::endl;
    std::cout << "\t Line: " << function.first << std::endl;

    pdb_instance.join();
    return 0;
}
