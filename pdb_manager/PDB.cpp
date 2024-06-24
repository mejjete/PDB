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
    
    pdb_instance.join();
    return 0;
}
