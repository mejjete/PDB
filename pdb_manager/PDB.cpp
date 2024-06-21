#include <concepts>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include "PDB.hpp"
#include "PDBDebugger.hpp"

bool is_signaled = false;

int main()
{
    using namespace pdb;
    
    PDBDebug pdb_instance("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3", 
        PDB_Debug_type::GDB);

    return 0;
}
