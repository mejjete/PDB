#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include <memory>
#include "PDB.hpp"

bool is_signaled = false;

int main()
{
    pdb::PDBDebug pdb_instance("mpirun -np 4", "./mpi_test.out", "arg1 arg2 arg3", 
        std::make_unique<pdb::GDBDebugger>());

    size_t pdb_size = pdb_instance.size();

    // Read initial print from debugger
    for(size_t i = 0; i < pdb_size; i++)
    {
        usleep(100000);
        auto &proc = pdb_instance.getProc(i);
        std::string main_buffer = proc.read();
        printf("%ld responded: \"%s\"\n", i, main_buffer.c_str());
    }

    while(!is_signaled)
    {
        printf("Command: ");

        std::string message;   
        getline(std::cin, message);
        message += "\n";

        // Broadcast message to all processes
        for(size_t i = 0; i < pdb_size; i++)
        {
            auto& proc = pdb_instance.getProc(i);
            proc.write(message);
        }

        usleep(10000);

        // Read responce from all processes
        for(size_t i = 0; i < pdb_size; i++)
        {
            auto& proc = pdb_instance.getProc(i);
            std::string main_buffer = proc.read();
            printf("%ld responded: \"%s\"\n", i, main_buffer.c_str());
        }
    }

    return 0;
}
